#include "music-blocker-filter.hpp"
#include <iostream>

#define SETTING_SENSITIVITY      "sensitivity"
#define SETTING_ACTION_MODE      "action_mode"
#define SETTING_DUCK_RATIO       "duck_ratio"
#define SETTING_CROSSFADE_MS     "crossfade_ms"
#define SETTING_REPLACEMENT_PATH "replacement_path"
#define SETTING_VOLUME           "replacement_volume"

static const char* filter_get_name(void* unused) {
    UNUSED_PARAMETER(unused);
    return "Music Blocker & Replacement Shield";
}

static void filter_update(void* data, obs_data_t* settings) {
    auto* filter = static_cast<music_blocker_filter*>(data);
    if (!filter || !settings) return;

    filter->sensitivity = static_cast<float>(obs_data_get_double(settings, SETTING_SENSITIVITY));
    filter->action_mode = static_cast<int>(obs_data_get_int(settings, SETTING_ACTION_MODE));
    filter->duck_ratio = static_cast<float>(obs_data_get_double(settings, SETTING_DUCK_RATIO));
    filter->crossfade_ms = static_cast<float>(obs_data_get_double(settings, SETTING_CROSSFADE_MS));
    filter->volume = static_cast<float>(obs_data_get_double(settings, SETTING_VOLUME));
    
    const char* path = obs_data_get_string(settings, SETTING_REPLACEMENT_PATH);
    std::string newPath = path ? path : "";

    if (filter->detector) {
        filter->detector->SetSensitivity(filter->sensitivity);
    }

    if (filter->player) {
        filter->player->SetCrossfadeDuration(filter->crossfade_ms);
        filter->player->SetVolume(filter->volume);

        if (newPath != filter->replacement_path && !newPath.empty()) {
            filter->replacement_path = newPath;
            filter->player->LoadWavFile(filter->replacement_path);
        }
    }
}

static void* filter_create(obs_data_t* settings, obs_source_t* source) {
    auto* filter = new music_blocker_filter();
    filter->context = source;
    filter->sample_rate = 48000;
    filter->channels = 2;
    filter->is_detected = false;

    filter->detector = std::make_unique<music_blocker::DSPMusicDetector>(filter->sample_rate, 1024);
    filter->player = std::make_unique<music_blocker::ReplacementAudioPlayer>(filter->sample_rate, filter->channels);

    filter_update(filter, settings);
    return filter;
}

static void filter_destroy(void* data) {
    auto* filter = static_cast<music_blocker_filter*>(data);
    if (filter) {
        delete filter;
    }
}

static struct obs_audio_data* filter_audio(void* data, struct obs_audio_data* audio) {
    auto* filter = static_cast<music_blocker_filter*>(data);
    if (!filter || !audio || audio->frames == 0) return audio;

    // Detect format update
    uint32_t currentSampleRate = audio_output_get_sample_rate(obs_get_audio());
    uint32_t currentChannels = audio_output_get_channels(obs_get_audio());

    if (currentSampleRate != filter->sample_rate || currentChannels != filter->channels) {
        filter->sample_rate = currentSampleRate > 0 ? currentSampleRate : 48000;
        filter->channels = currentChannels > 0 ? currentChannels : 2;
        filter->detector->SetSampleRate(filter->sample_rate);
    }

    // Downmix first channel for DSP spectral music detection
    float* channelData = reinterpret_cast<float*>(audio->data[0]);
    filter->is_detected = filter->detector->ProcessSamples(channelData, audio->frames);

    // Apply audio replacement / ducking crossfade
    float effectiveDuckRatio = (filter->action_mode == 1) ? filter->duck_ratio : 0.0f;
    bool enableReplacement = (filter->action_mode != 2);

    if (enableReplacement) {
        float* audioChannels[MAX_AV_PLANES];
        for (size_t i = 0; i < filter->channels; ++i) {
            audioChannels[i] = reinterpret_cast<float*>(audio->data[i]);
        }

        filter->player->ProcessAudio(audioChannels, filter->channels, audio->frames, filter->is_detected, effectiveDuckRatio);
    } else if (filter->is_detected) {
        // Mute only mode
        for (size_t i = 0; i < filter->channels; ++i) {
            float* chData = reinterpret_cast<float*>(audio->data[i]);
            for (size_t f = 0; f < audio->frames; ++f) {
                chData[f] *= effectiveDuckRatio;
            }
        }
    }

    return audio;
}

static obs_properties_t* filter_properties(void* data) {
    obs_properties_t* props = obs_properties_create();

    obs_properties_add_float_slider(props, SETTING_SENSITIVITY, "Music Detection Sensitivity", 0.0, 1.0, 0.05);

    obs_property_t* modeList = obs_properties_add_list(
        props, SETTING_ACTION_MODE, "Action on Music Detect",
        OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT
    );
    obs_property_list_add_int(modeList, "Replace & Mute Original", 0);
    obs_property_list_add_int(modeList, "Duck Original + Mix Custom Music", 1);
    obs_property_list_add_int(modeList, "Mute Original Only", 2);

    obs_properties_add_float_slider(props, SETTING_DUCK_RATIO, "Duck Audio Ratio", 0.0, 0.5, 0.05);
    obs_properties_add_float_slider(props, SETTING_CROSSFADE_MS, "Crossfade Speed (ms)", 50.0, 2000.0, 50.0);
    obs_properties_add_float_slider(props, SETTING_VOLUME, "Replacement Music Volume", 0.0, 1.0, 0.05);

    obs_properties_add_path(
        props, SETTING_REPLACEMENT_PATH, "Replacement Audio File (.wav)",
        OBS_PATH_FILE, "Audio Files (*.wav)", nullptr
    );

    UNUSED_PARAMETER(data);
    return props;
}

static void filter_defaults(obs_data_t* settings) {
    obs_data_set_default_double(settings, SETTING_SENSITIVITY, 0.6);
    obs_data_set_default_int(settings, SETTING_ACTION_MODE, 0);
    obs_data_set_default_double(settings, SETTING_DUCK_RATIO, 0.1);
    obs_data_set_default_double(settings, SETTING_CROSSFADE_MS, 300.0);
    obs_data_set_default_double(settings, SETTING_VOLUME, 0.8);
    obs_data_set_default_string(settings, SETTING_REPLACEMENT_PATH, "");
}

struct obs_source_info music_blocker_filter_info = {
    .id           = "obs_music_blocker_filter",
    .type         = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_AUDIO,
    .get_name     = filter_get_name,
    .create       = filter_create,
    .destroy      = filter_destroy,
    .update       = filter_update,
    .get_defaults = filter_defaults,
    .get_properties = filter_properties,
    .filter_audio = filter_audio,
};
