# OBS Music Blocker & Replacement Plugin (`obs-music-blocker`)

An advanced OBS Studio plugin and script designed to detect background music during live streams, block/mute the copyrighted audio, and seamlessly replace it with your custom royalty-free music track in real time.

---

## Features

- **Real-Time Music Detection (DSP)**:
  Uses spectral harmonic analysis (FFT, spectral flatness, harmonic-to-noise ratio) to distinguish background music from voice/speech.
- **Seamless Crossfade & Audio Replacement**:
  Smoothly ducks/mutes the stream audio input (e.g. 300ms crossfade) and blends in a custom replacement `.wav` music track or fallback ambient pad.
- **OBS Studio UI Integration**:
  Provides configurable sliders for Sensitivity, Action Modes (*Replace & Mute*, *Duck Original*, *Mute Only*), Crossfade Speed, Volume, and File Selection directly inside OBS source filter properties.
- **Dual Option (Native C++ Plugin + OBS Python Script)**:
  Includes both the high-performance native C++ `.dll` filter plugin and an instant Python script for OBS Studio (**Tools -> Scripts**).

---

## Repository Structure

```
.
├── CMakeLists.txt                 # CMake build configuration for C++ plugin
├── README.md                      # Setup and usage guide
├── scripts/
│   └── obs_music_blocker.py       # OBS Python script fallback
└── src/
    ├── plugin-main.cpp            # OBS Module load/unload entry points
    ├── music-blocker-filter.hpp   # OBS Filter source definition & properties
    ├── music-blocker-filter.cpp   # OBS Filter audio processing callback
    ├── dsp-detector.hpp           # Spectral FFT analysis header
    ├── dsp-detector.cpp           # FFT & music detection logic
    ├── audio-resampler.hpp        # Replacement audio player & crossfader header
    └── audio-resampler.cpp        # WAV reader & frame crossfading
```

---

## Installation & Setup Options

### Option 1: Native C++ Plugin Compilation

#### Requirements:
- CMake (3.16+)
- C++17 compiler (Visual Studio 2019/2022 on Windows, GCC/Clang on Linux)
- [OBS Studio Development Headers](https://github.com/obsproject/obs-studio) (`libobs`)

#### Build Steps (Windows):
```powershell
# 1. Create build directory
mkdir build
cd build

# 2. Configure CMake (specify path to your OBS Studio installation if needed)
cmake -DOBS_STUDIO_DIR="C:/Program Files/obs-studio" ..

# 3. Build project
cmake --build . --config Release

# 4. Install plugin into OBS Studio
# Copy the compiled obs-music-blocker.dll into:
# C:\Program Files\obs-studio\obs-plugins\64bit\
```

#### How to Use in OBS Studio:
1. Open **OBS Studio**.
2. Right-click your **Audio Source** (Desktop Audio, Game Capture, Mic) -> **Filters**.
3. Click **+** under Audio/Video Filters and select **Music Blocker & Replacement Shield**.
4. Configure your **Sensitivity**, select your custom replacement `.wav` audio track, and set your desired **Action Mode**.

---

### Option 2: Standalone OBS Python Script (No Compilation Required)

If you want an immediate setup without compiling C++ binaries:

1. Open **OBS Studio**.
2. Go to **Tools** -> **Scripts**.
3. Click **+** (Add Script) and select `scripts/obs_music_blocker.py`.
4. In the script configuration panel:
   - Select your **Stream Audio Source** (to block/duck).
   - Select your **Custom Music Replacement Source** (Media Source playing your royalty-free music).
   - Set Ducking Volume level (dB).
5. (Optional) Assign a hotkey in **Settings -> Hotkeys -> Toggle OBS Music Shield**.

---

## Filter Settings Reference

| Property | Description | Default |
| :--- | :--- | :--- |
| **Sensitivity** | Detection threshold for music spectral harmonics | `0.6` |
| **Action Mode** | *Replace & Mute Original*, *Duck Original + Mix*, *Mute Only* | `Replace & Mute` |
| **Duck Audio Ratio** | Volume attenuation level when ducking original stream | `0.1 (-20dB)` |
| **Crossfade Speed (ms)** | Fade-in/out transition speed for audio replacement | `300 ms` |
| **Replacement Volume** | Volume output level for custom replacement track | `0.8` |
| **Replacement Audio File** | Path to custom `.wav` music track to play when blocked | `(Fallback Pad)` |

---

## License

MIT License. Designed for live streamers, content creators, and broadcast software developers.
