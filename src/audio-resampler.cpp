#include "audio-resampler.hpp"
#include <fstream>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace music_blocker {

#pragma pack(push, 1)
struct WavHeader {
    char riff[4];           // "RIFF"
    uint32_t chunkSize;
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmtSize;
    uint16_t audioFormat;   // 1 = PCM, 3 = IEEE Float
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];           // "data"
    uint32_t dataSize;
};
#pragma pack(pop)

ReplacementAudioPlayer::ReplacementAudioPlayer(uint32_t outputSampleRate, uint32_t channels)
    : m_sampleRate(outputSampleRate),
      m_channels(channels),
      m_volume(0.8f),
      m_crossfadeMs(300.0f),
      m_playbackPosition(0),
      m_currentFade(0.0f)
{
    GenerateFallbackAudio(10.0f);
}

void ReplacementAudioPlayer::SetCrossfadeDuration(float durationMs) {
    m_crossfadeMs = std::max(50.0f, durationMs);
}

void ReplacementAudioPlayer::SetVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);
}

void ReplacementAudioPlayer::GenerateFallbackAudio(float durationSec) {
    size_t totalFrames = static_cast<size_t>(m_sampleRate * durationSec);
    m_replacementBuffer.resize(totalFrames * m_channels);

    // Create a pleasant, non-obtrusive synth chord loop (A minor chord pad: A4, C5, E5)
    float freqs[] = { 440.0f, 523.25f, 659.25f };
    size_t numFreqs = 3;

    for (size_t frame = 0; frame < totalFrames; ++frame) {
        float t = static_cast<float>(frame) / m_sampleRate;
        float sample = 0.0f;
        for (size_t k = 0; k < numFreqs; ++k) {
            sample += 0.15f * std::sin(2.0f * static_cast<float>(M_PI) * freqs[k] * t);
        }
        // Smooth loop envelope
        float envelope = 0.8f + 0.2f * std::sin(2.0f * static_cast<float>(M_PI) * 0.1f * t);
        sample *= envelope;

        for (uint32_t ch = 0; ch < m_channels; ++ch) {
            m_replacementBuffer[frame * m_channels + ch] = sample;
        }
    }
    m_playbackPosition = 0;
}

bool ReplacementAudioPlayer::LoadWavFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[obs-music-blocker] Failed to open WAV file: " << filePath << std::endl;
        return false;
    }

    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    if (std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
        std::cerr << "[obs-music-blocker] Invalid WAV file header format." << std::endl;
        return false;
    }

    std::vector<uint8_t> rawData(header.dataSize);
    file.read(reinterpret_cast<char*>(rawData.data()), header.dataSize);

    size_t numSamples = header.dataSize / (header.bitsPerSample / 8);
    size_t numFrames = numSamples / header.numChannels;

    m_replacementBuffer.clear();
    m_replacementBuffer.resize(numFrames * m_channels);

    if (header.bitsPerSample == 16) {
        const int16_t* pcm16 = reinterpret_cast<const int16_t*>(rawData.data());
        for (size_t frame = 0; frame < numFrames; ++frame) {
            for (uint32_t ch = 0; ch < m_channels; ++ch) {
                uint32_t srcCh = std::min(ch, static_cast<uint32_t>(header.numChannels - 1));
                float normSample = pcm16[frame * header.numChannels + srcCh] / 32768.0f;
                m_replacementBuffer[frame * m_channels + ch] = normSample;
            }
        }
    } else if (header.bitsPerSample == 32 && header.audioFormat == 3) {
        const float* float32 = reinterpret_cast<const float*>(rawData.data());
        for (size_t frame = 0; frame < numFrames; ++frame) {
            for (uint32_t ch = 0; ch < m_channels; ++ch) {
                uint32_t srcCh = std::min(ch, static_cast<uint32_t>(header.numChannels - 1));
                m_replacementBuffer[frame * m_channels + ch] = float32[frame * header.numChannels + srcCh];
            }
        }
    } else {
        std::cerr << "[obs-music-blocker] Unsupported WAV format (requires 16-bit PCM or 32-bit Float)." << std::endl;
        GenerateFallbackAudio(10.0f);
        return false;
    }

    m_playbackPosition = 0;
    std::cout << "[obs-music-blocker] Successfully loaded WAV track: " << filePath << " (" << numFrames << " frames)" << std::endl;
    return true;
}

void ReplacementAudioPlayer::ProcessAudio(float** audioData, uint32_t channels, uint32_t frames, bool blockMusic, float duckRatio) {
    if (!audioData || frames == 0 || m_replacementBuffer.empty()) return;

    size_t totalBufferFrames = m_replacementBuffer.size() / m_channels;
    float fadeStep = 1.0f / (m_sampleRate * (m_crossfadeMs / 1000.0f));

    for (size_t frame = 0; frame < frames; ++frame) {
        // Target fade state: 1.0 if music blocked, 0.0 if normal audio stream
        float targetFade = blockMusic ? 1.0f : 0.0f;

        if (m_currentFade < targetFade) {
            m_currentFade = std::min(m_currentFade + fadeStep, targetFade);
        } else if (m_currentFade > targetFade) {
            m_currentFade = std::max(m_currentFade - fadeStep, targetFade);
        }

        // Fetch replacement audio frame samples
        size_t currentReadPos = (m_playbackPosition + frame) % totalBufferFrames;

        for (uint32_t ch = 0; ch < channels; ++ch) {
            uint32_t bufCh = std::min(ch, m_channels - 1);
            float replacementSample = m_replacementBuffer[currentReadPos * m_channels + bufCh] * m_volume;

            float originalSample = audioData[ch][frame];

            // Attenuate original sample when ducked/blocked
            float originalGain = (1.0f - m_currentFade) + (m_currentFade * duckRatio);

            // Blend original stream audio and replacement audio
            audioData[ch][frame] = (originalSample * originalGain) + (replacementSample * m_currentFade);
        }
    }

    // Advance replacement track playhead
    m_playbackPosition = (m_playbackPosition + frames) % totalBufferFrames;
}

} // namespace music_blocker
