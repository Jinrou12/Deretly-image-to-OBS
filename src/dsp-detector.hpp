#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <complex>

namespace music_blocker {

class DSPMusicDetector {
public:
    DSPMusicDetector(uint32_t sampleRate = 48000, size_t fftSize = 1024);
    ~DSPMusicDetector() = default;

    // Reset internal state and buffers
    void Reset();

    // Configure detection parameters
    void SetSensitivity(float sensitivity); // 0.0 (low) to 1.0 (high)
    void SetSampleRate(uint32_t sampleRate);

    // Push new PCM audio samples (single channel or averaged)
    // Returns true if music is currently detected
    bool ProcessSamples(const float* samples, size_t count);

    // Get current music detection score (0.0 to 1.0)
    float GetConfidenceScore() const { return m_lastConfidence; }

private:
    void ExecuteFFT(std::vector<std::complex<float>>& buffer);
    float ComputeSpectralFlatness(const std::vector<float>& magnitude);
    float ComputeSpectralFlux(const std::vector<float>& magnitude);
    float ComputeHarmonicRatio(const std::vector<float>& magnitude);

    uint32_t m_sampleRate;
    size_t m_fftSize;
    float m_sensitivity;

    std::vector<float> m_inputBuffer;
    std::vector<float> m_previousMagnitude;
    
    float m_lastConfidence;
    int m_consecutiveDetectCount;
    int m_consecutiveSilenceCount;
    bool m_isMusicDetected;
};

} // namespace music_blocker
