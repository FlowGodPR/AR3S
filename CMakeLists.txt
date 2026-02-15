cmake_minimum_required(VERSION 3.20)
project(AR3S VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")

# JUCE path
set(JUCE_PATH "$ENV{HOME}/JUCE")

# Add JUCE with EXCLUDE_FROM_ALL to avoid C compiler issues
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE-build EXCLUDE_FROM_ALL)

# =============================================================================
# AR3S Master Plugin
# =============================================================================
juce_add_plugin(AR3S
    PLUGIN_NAME "AR3S"
    PLUGIN_MANUFACTURER "ARES"
    PLUGIN_MANUFACTURER_CODE "Ares"
    PLUGIN_CODE "Ar3M"
    FORMATS AU VST3 Standalone
    DISPLAY_SPLASH_SCREEN FALSE
    PLUGIN_IS_SYNTH FALSE
)

target_sources(AR3S PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ThemeData.h
    Source/Localization.h
    Source/SharedMemory.h
)

target_compile_definitions(AR3S PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AR3S PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
)

# =============================================================================
# AR3S Satellite Plugin
# =============================================================================
juce_add_plugin(AR3SSatellite
    PLUGIN_NAME "AR3S Satellite"
    PLUGIN_MANUFACTURER "ARES"
    PLUGIN_MANUFACTURER_CODE "Ares"
    PLUGIN_CODE "Ar3S"
    FORMATS AU VST3 Standalone
    DISPLAY_SPLASH_SCREEN FALSE
    PLUGIN_IS_SYNTH FALSE
)

target_sources(AR3SSatellite PRIVATE
    Source/SatelliteProcessor.cpp
    Source/SatelliteEditor.cpp
    Source/ThemeData.h
    Source/SharedMemory.h
)

target_compile_definitions(AR3SSatellite PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AR3SSatellite PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
)

# Product directory
set_target_properties(AR3S AR3SSatellite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)