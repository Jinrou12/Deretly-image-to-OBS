obs = obslua

local timestamp_path = "D:/Users/.gemini/antigravity/scratch/PC/saved_images/latest_timestamp.txt"
local image_path = "D:/Users/.gemini/antigravity/scratch/PC/saved_images/latest_clipboard.png"
local source_name = "Clipboard Image"
local last_timestamp = ""

function script_description()
    return "Auto Clipboard Image Paster\nOnly shows image in OBS when a NEW image is copied. If you delete the image in OBS, it stays deleted until you copy a new image!"
end

function check_and_update()
    -- Read current timestamp from file
    local f = io.open(timestamp_path, "r")
    if not f then return end
    local current_timestamp = f:read("*all")
    f:close()

    -- If no timestamp or timestamp hasn't changed (no new image copied), DO NOTHING!
    -- This ensures if you delete the image in OBS, it STAYS DELETED!
    if not current_timestamp or current_timestamp == "" or current_timestamp == last_timestamp then
        return
    end

    last_timestamp = current_timestamp

    local current_scene_source = obs.obs_frontend_get_current_scene()
    if not current_scene_source then return end
    
    local scene = obs.obs_scene_from_source(current_scene_source)
    if not scene then
        obs.obs_source_release(current_scene_source)
        return
    end

    local existing_source = obs.obs_get_source_by_name(source_name)
    if not existing_source then
        -- Auto-create image source in active OBS scene ONLY when a new image is copied
        local settings = obs.obs_data_create()
        obs.obs_data_set_string(settings, "file", image_path)
        obs.obs_data_set_bool(settings, "unload", true)
        
        local new_source = obs.obs_source_create("image_source", source_name, settings, nil)
        obs.obs_scene_add(scene, new_source)
        
        obs.obs_data_release(settings)
        obs.obs_source_release(new_source)
    else
        -- Update existing source settings
        local settings = obs.obs_source_get_settings(existing_source)
        obs.obs_data_set_string(settings, "file", image_path)
        obs.obs_source_update(existing_source, settings)
        obs.obs_data_release(settings)
        obs.obs_source_release(existing_source)
    end
    
    obs.obs_source_release(current_scene_source)
end

function script_load(settings)
    -- Read initial timestamp so existing image isn't re-added on script startup
    local f = io.open(timestamp_path, "r")
    if f then
        last_timestamp = f:read("*all") or ""
        f:close()
    end
    obs.timer_add(check_and_update, 1000)
end
