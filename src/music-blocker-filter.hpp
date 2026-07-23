#pragma once

#include <obs-module.h>
#include "dsp-detector.hpp"
#include "audio-resampler.hpp"
#include <string>
#include <memory>

struct music_blocker_filter {
    obs_source_t* context;

    // Filter settings
    float sensitivity;          // 0.0 to 1.0
    int action_mode;            // 0 = Replace & Mute Original, 1 = Duck Original + Mix, 2 = Mute Only
    float duck_ratio;           // 0.0 (mute) to 0.5 (-6dB)
    float crossfade_ms;         // Fade duration in ms
    float volume;               // Replacement track volume (0.0 to 1.0)
    std::string replacement_path;

    // DSP & Audio engine instances
    std::unique_ptr<music_blocker::DSPMusicDetector> detector;
    std::unique_ptr<music_blocker::ReplacementAudioPlayer> player;

    uint32_t sample_rate;
    uint32_t channels;
    bool is_detected;
};

// Register OBS source info definition
extern struct obs_source_info music_blocker_filter_info;
