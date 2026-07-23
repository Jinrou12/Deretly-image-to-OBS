#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace music_blocker {

class ReplacementAudioPlayer {
public:
    ReplacementAudioPlayer(uint32_t outputSampleRate = 48000, uint32_t channels = 2);
    ~ReplacementAudioPlayer() = default;

    // Load replacement audio track (supports PCM WAV format)
    bool LoadWavFile(const std::string& filePath);

    // Generate fallback synthesized ambient/chill track if no file provided
    void GenerateFallbackAudio(float durationSec = 10.0f);

    // Set crossfade transition time in milliseconds
    void SetCrossfadeDuration(float durationMs);

    // Configure playback volume (0.0 to 1.0)
    void SetVolume(float volume);

    // Process input buffer and perform crossfade with replacement track
    // targetState: true = Block Music (Play Replacement), false = Normal Stream (Original Audio)
    void ProcessAudio(float** audioData, uint32_t channels, uint32_t frames, bool blockMusic, float duckRatio = 0.0f);

private:
    uint32_t m_sampleRate;
    uint32_t m_channels;
    float m_volume;
    float m_crossfadeMs;

    // Replacement audio PCM buffer (interleaved stereo float)
    std::vector<float> m_replacementBuffer;
    size_t m_playbackPosition;

    // Current crossfade state (0.0 = full original stream, 1.0 = full replacement track)
    float m_currentFade;
};

} // namespace music_blocker
