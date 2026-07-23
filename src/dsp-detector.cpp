#include "dsp-detector.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace music_blocker {

DSPMusicDetector::DSPMusicDetector(uint32_t sampleRate, size_t fftSize)
    : m_sampleRate(sampleRate),
      m_fftSize(fftSize),
      m_sensitivity(0.5f),
      m_lastConfidence(0.0f),
      m_consecutiveDetectCount(0),
      m_consecutiveSilenceCount(0),
      m_isMusicDetected(false)
{
    m_inputBuffer.reserve(m_fftSize * 2);
    m_previousMagnitude.resize(m_fftSize / 2, 0.0f);
}

void DSPMusicDetector::Reset() {
    m_inputBuffer.clear();
    std::fill(m_previousMagnitude.begin(), m_previousMagnitude.end(), 0.0f);
    m_lastConfidence = 0.0f;
    m_consecutiveDetectCount = 0;
    m_consecutiveSilenceCount = 0;
    m_isMusicDetected = false;
}

void DSPMusicDetector::SetSensitivity(float sensitivity) {
    m_sensitivity = std::clamp(sensitivity, 0.0f, 1.0f);
}

void DSPMusicDetector::SetSampleRate(uint32_t sampleRate) {
    if (sampleRate != m_sampleRate && sampleRate > 0) {
        m_sampleRate = sampleRate;
        Reset();
    }
}

// Radix-2 Cooley-Tukey FFT
void DSPMusicDetector::ExecuteFFT(std::vector<std::complex<float>>& buffer) {
    size_t n = buffer.size();
    if (n <= 1) return;

    // Bit reversal permutation
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i < j) {
            std::swap(buffer[i], buffer[j]);
        }
        size_t bit = n >> 1;
        while (j >= bit && bit > 0) {
            j -= bit;
            bit >>= 1;
        }
        j += bit;
    }

    // Cooley-Tukey calculation
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * static_cast<float>(M_PI) / len;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));
        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                std::complex<float> u = buffer[i + k];
                std::complex<float> v = buffer[i + k + len / 2] * w;
                buffer[i + k] = u + v;
                buffer[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

float DSPMusicDetector::ComputeSpectralFlatness(const std::vector<float>& magnitude) {
    // Geometric mean / Arithmetic mean of spectrum
    double logSum = 0.0;
    double sum = 0.0;
    size_t count = magnitude.size();

    if (count == 0) return 1.0f;

    for (float mag : magnitude) {
        float val = std::max(mag, 1e-7f);
        logSum += std::log(val);
        sum += val;
    }

    double arithmeticMean = sum / count;
    double geometricMean = std::exp(logSum / count);

    if (arithmeticMean <= 1e-7) return 1.0f;
    return static_cast<float>(geometricMean / arithmeticMean);
}

float DSPMusicDetector::ComputeSpectralFlux(const std::vector<float>& magnitude) {
    float flux = 0.0f;
    size_t count = std::min(magnitude.size(), m_previousMagnitude.size());

    for (size_t i = 0; i < count; ++i) {
        float diff = magnitude[i] - m_previousMagnitude[i];
        if (diff > 0.0f) {
            flux += diff;
        }
    }
    return flux / count;
}

float DSPMusicDetector::ComputeHarmonicRatio(const std::vector<float>& magnitude) {
    // Detect sharp harmonic peaks (typical in musical instruments/tunes vs broadband noise)
    float maxPeak = 0.0f;
    float avgEnergy = 0.0f;

    for (float mag : magnitude) {
        maxPeak = std::max(maxPeak, mag);
        avgEnergy += mag;
    }

    if (avgEnergy <= 1e-6f) return 0.0f;
    avgEnergy /= magnitude.size();

    return std::min(maxPeak / (avgEnergy + 1e-6f), 10.0f) / 10.0f;
}

bool DSPMusicDetector::ProcessSamples(const float* samples, size_t count) {
    if (!samples || count == 0) return m_isMusicDetected;

    m_inputBuffer.insert(m_inputBuffer.end(), samples, samples + count);

    while (m_inputBuffer.size() >= m_fftSize) {
        // Window preparation (Hann window)
        std::vector<std::complex<float>> fftBuffer(m_fftSize);
        for (size_t i = 0; i < m_fftSize; ++i) {
            float window = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * i / (m_fftSize - 1)));
            fftBuffer[i] = std::complex<float>(m_inputBuffer[i] * window, 0.0f);
        }

        ExecuteFFT(fftBuffer);

        // Calculate half-spectrum magnitudes
        size_t halfSize = m_fftSize / 2;
        std::vector<float> magnitude(halfSize);
        for (size_t i = 0; i < halfSize; ++i) {
            magnitude[i] = std::abs(fftBuffer[i]);
        }

        float flatness = ComputeSpectralFlatness(magnitude); // Lower for tonal/music, higher for noise/speech
        float harmonicity = ComputeHarmonicRatio(magnitude); // Higher for pitch/instruments

        // Music indicator score (0.0 to 1.0)
        // Music generally exhibits low spectral flatness (tonal structure) + high harmonic ratio
        float score = (1.0f - flatness) * 0.5f + harmonicity * 0.5f;

        // Dynamic threshold adjusted by user sensitivity slider (0.0 to 1.0)
        float threshold = 0.65f - (m_sensitivity * 0.35f);

        m_lastConfidence = score;
        m_previousMagnitude = magnitude;

        if (score >= threshold) {
            m_consecutiveDetectCount++;
            m_consecutiveSilenceCount = 0;
        } else {
            m_consecutiveSilenceCount++;
            if (m_consecutiveSilenceCount > 5) {
                m_consecutiveDetectCount = 0;
            }
        }

        // Require at least 3 consecutive FFT windows exceeding threshold (~60ms) to trigger
        if (m_consecutiveDetectCount >= 3) {
            m_isMusicDetected = true;
        } else if (m_consecutiveSilenceCount >= 10) {
            m_isMusicDetected = false;
        }

        // Shift window by 50% overlap (hop size = fftSize / 2)
        m_inputBuffer.erase(m_inputBuffer.begin(), m_inputBuffer.begin() + (m_fftSize / 2));
    }

    return m_isMusicDetected;
}

} // namespace music_blocker
