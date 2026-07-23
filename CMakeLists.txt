cmake_minimum_required(VERSION 3.16)
project(obs-music-blocker VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find OBS Studio development headers/libraries
# Users can specify -DOBS_STUDIO_DIR="<path_to_obs_studio>" if not in default location
find_path(OBS_INCLUDE_DIR "obs-module.h"
    PATHS
    "${OBS_STUDIO_DIR}/libobs"
    "C:/Program Files/obs-studio/include"
    "C:/obs-studio/libobs"
    "/usr/include/obs"
    "/usr/local/include/obs"
)

find_library(LIBOBS_LIB
    NAMES obs libobs
    PATHS
    "${OBS_STUDIO_DIR}/libobs/Release"
    "${OBS_STUDIO_DIR}/bin/64bit"
    "C:/Program Files/obs-studio/bin/64bit"
    "C:/obs-studio/bin/64bit"
    "/usr/lib"
    "/usr/local/lib"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(OBS_INCLUDE_DIR)
    include_directories(${OBS_INCLUDE_DIR})
endif()

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.hpp"
)

add_library(obs-music-blocker SHARED ${SOURCES})

if(LIBOBS_LIB)
    target_link_libraries(obs-music-blocker PRIVATE ${LIBOBS_LIB})
endif()

# Platform specific configurations
if(MSVC)
    target_compile_options(obs-music-blocker PRIVATE /W4 /WX-)
    set_target_properties(obs-music-blocker PROPERTIES
        PREFIX ""
        OUTPUT_NAME "obs-music-blocker"
    )
else()
    target_compile_options(obs-music-blocker PRIVATE -Wall -Wextra)
endif()

# Installation target
install(TARGETS obs-music-blocker
    LIBRARY DESTINATION "obs-plugins/64bit"
    RUNTIME DESTINATION "obs-plugins/64bit"
)
