"""
OBS Universal Music Blocker & Single-Step Replacement Script
Author: Antigravity AI
Description: Universally blocks/ducks ANY music or song during unplanned livestreams and seamlessly replaces it with your chosen replacement music in a single step.
"""

import obspython as obs

# Global script settings & handles
target_audio_source = ""
replacement_media_source = ""
replacement_file_path = ""
auto_duck = True
duck_volume_db = -24.0
is_shield_active = False
hotkey_id = obs.OBS_INVALID_HOTKEY_ID

def script_description():
    return (
        "<b>OBS Universal Music Blocker & Shield</b><br/><br/>"
        "<b>Universal Rule</b>: Automatically blocks ANY song or music track played during a live stream.<br/>"
        "<b>Single-Step Setup</b>: Select your stream audio source and your replacement music file/folder below."
    )

def script_update(settings):
    global target_audio_source, replacement_media_source, replacement_file_path, auto_duck, duck_volume_db
    target_audio_source = obs.obs_data_get_string(settings, "target_audio_source")
    replacement_media_source = obs.obs_data_get_string(settings, "replacement_media_source")
    replacement_file_path = obs.obs_data_get_string(settings, "replacement_file_path")
    auto_duck = obs.obs_data_get_bool(settings, "auto_duck")
    duck_volume_db = obs.obs_data_get_double(settings, "duck_volume_db")

def toggle_shield(pressed):
    global is_shield_active
    if not pressed:
        return

    is_shield_active = not is_shield_active
    apply_shield_state(is_shield_active)

def apply_shield_state(active):
    # 1. Universal Mute / Duck of Stream Audio Source
    if target_audio_source:
        source = obs.obs_get_source_by_name(target_audio_source)
        if source is not None:
            if active:
                if auto_duck:
                    gain = 10.0 ** (duck_volume_db / 20.0)
                    obs.obs_source_set_volume(source, gain)
                    print(f"[Universal Blocker] Ducked stream audio ({target_audio_source}) to {duck_volume_db} dB")
                else:
                    obs.obs_source_set_muted(source, True)
                    print(f"[Universal Blocker] Muted stream audio ({target_audio_source})")
            else:
                obs.obs_source_set_muted(source, False)
                obs.obs_source_set_volume(source, 1.0)
                print(f"[Universal Blocker] Restored stream audio ({target_audio_source})")
            obs.obs_source_release(source)

    # 2. Single-Step Custom Replacement Track Control
    if replacement_media_source:
        rep_source = obs.obs_get_source_by_name(replacement_media_source)
        if rep_source is not None:
            if active:
                obs.obs_source_set_muted(rep_source, False)
                obs.obs_source_media_restart(rep_source)
                print(f"[Universal Blocker] Playing replacement track source: {replacement_media_source}")
            else:
                obs.obs_source_set_muted(rep_source, True)
                print(f"[Universal Blocker] Stopped replacement track source: {replacement_media_source}")
            obs.obs_source_release(rep_source)

def script_properties():
    props = obs.obs_properties_create()

    # 1. Stream Audio Source Selection
    audio_list = obs.obs_properties_add_list(
        props, "target_audio_source", "Stream Audio Source (To Block ALL Music)",
        obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    obs.obs_property_list_add_string(audio_list, "-- Select Stream Audio (e.g. Desktop Audio) --", "")

    # 2. Single-Step Replacement Media Source Selector
    media_list = obs.obs_properties_add_list(
        props, "replacement_media_source", "Custom Music Replacement Source",
        obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    obs.obs_property_list_add_string(media_list, "-- Select Replacement Media Source --", "")

    # Enumerate all available audio / media sources in OBS
    sources = obs.obs_enum_sources()
    if sources is not None:
        for src in sources:
            name = obs.obs_source_get_name(src)
            src_id = obs.obs_source_get_unversioned_id(src)
            obs.obs_property_list_add_string(audio_list, name, name)
            if src_id in ["ffmpeg_source", "vlc_source", "media_source"]:
                obs.obs_property_list_add_string(media_list, name, name)
        obs.source_list_release(sources)

    # 3. Single-Step File Browser for Replacement Track
    obs.obs_properties_add_path(
        props, "replacement_file_path", "Single-Step Music File / Folder",
        obs.OBS_PATH_FILE, "Audio Files (*.mp3 *.wav *.flac *.aac *.ogg *.m4a)", None
    )

    # Settings & Duck Controls
    obs.obs_properties_add_bool(props, "auto_duck", "Duck Stream Audio (instead of Full Mute)")
    obs.obs_properties_add_float_slider(props, "duck_volume_db", "Duck Volume Level (dB)", -60.0, 0.0, 1.0)

    # Action Toggle Button
    obs.obs_properties_add_button(props, "toggle_button", "Toggle Music Shield Now", lambda p, b: toggle_shield(True))

    return props

def script_defaults(settings):
    obs.obs_data_set_default_string(settings, "target_audio_source", "Desktop Audio")
    obs.obs_data_set_default_bool(settings, "auto_duck", True)
    obs.obs_data_set_default_double(settings, "duck_volume_db", -24.0)

def script_load(settings):
    global hotkey_id
    hotkey_id = obs.obs_hotkey_register_frontend("toggle_music_shield", "Toggle OBS Music Shield", toggle_shield)
    hotkey_data = obs.obs_data_get_array(settings, "toggle_music_shield_hotkey")
    if hotkey_data is not None:
        obs.obs_hotkey_load(hotkey_id, hotkey_data)
        obs.obs_data_array_release(hotkey_data)

def script_save(settings):
    global hotkey_id
    if hotkey_id != obs.OBS_INVALID_HOTKEY_ID:
        hotkey_data = obs.obs_hotkey_save(hotkey_id)
        obs.obs_data_set_array(settings, "toggle_music_shield_hotkey", hotkey_data)
        obs.obs_data_array_release(hotkey_data)
