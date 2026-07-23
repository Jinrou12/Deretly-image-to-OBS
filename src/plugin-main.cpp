#include <obs-module.h>
#include "music-blocker-filter.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Antigravity OBS Plugin Team")
OBS_MODULE_USE_DEFAULT_LOCALE("obs-music-blocker", "en-US")

bool obs_module_load(void) {
    obs_register_source(&music_blocker_filter_info);
    blog(LOG_INFO, "[obs-music-blocker] Plugin loaded successfully!");
    return true;
}

void obs_module_unload(void) {
    blog(LOG_INFO, "[obs-music-blocker] Plugin unloaded.");
}
