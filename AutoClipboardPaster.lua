obs = obslua

local hotkey_id = obs.OBS_INVALID_HOTKEY_ID
local image_path = "D:/Users/.gemini/antigravity/scratch/PC/saved_images/latest_clipboard.png"
local source_name = "Clipboard Image"

function script_description()
    return "Paste Clipboard Image on Ctrl+V\nOnly pastes/updates the copied image in OBS when you press Ctrl+V inside OBS."
end

function paste_clipboard_image(pressed)
    if not pressed then return end

    local f = io.open(image_path, "r")
    if not f then return end
    f:close()

    local current_scene_source = obs.obs_frontend_get_current_scene()
    if not current_scene_source then return end
    
    local scene = obs.obs_scene_from_source(current_scene_source)
    if not scene then
        obs.obs_source_release(current_scene_source)
        return
    end

    local existing_source = obs.obs_get_source_by_name(source_name)
    if not existing_source then
        -- Auto-create image source in active OBS scene ONCE when Ctrl+V is pressed
        local settings = obs.obs_data_create()
        obs.obs_data_set_string(settings, "file", image_path)
        obs.obs_data_set_bool(settings, "unload", true)
        
        local new_source = obs.obs_source_create("image_source", source_name, settings, nil)
        obs.obs_scene_add(scene, new_source)
        
        obs.obs_data_release(settings)
        obs.obs_source_release(new_source)
    else
        -- Force refresh existing image source on Ctrl+V
        local settings = obs.obs_source_get_settings(existing_source)
        obs.obs_data_set_string(settings, "file", image_path)
        obs.obs_source_update(existing_source, settings)
        obs.obs_data_release(settings)
        obs.obs_source_release(existing_source)
    end
    
    obs.obs_source_release(current_scene_source)
end

function script_load(settings)
    hotkey_id = obs.obs_hotkey_register_frontend("paste_clipboard_image_hotkey", "Paste Clipboard Image", paste_clipboard_image)
    local hotkey_array = obs.obs_data_get_array(settings, "paste_clipboard_image_hotkey")
    obs.obs_hotkey_load(hotkey_id, hotkey_array)
    obs.obs_data_array_release(hotkey_array)
end

function script_save(settings)
    local hotkey_array = obs.obs_hotkey_save(hotkey_id)
    obs.obs_data_set_array(settings, "paste_clipboard_image_hotkey", hotkey_array)
    obs.obs_data_array_release(hotkey_array)
end
