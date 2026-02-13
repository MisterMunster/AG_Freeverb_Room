cmake_minimum_required(VERSION 3.22)

project(AntigravPlateReverb VERSION 0.1.0 LANGUAGES C CXX)

# Set standard C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# -----------------------------------------------------------------------------
# Fetch JUCE
# -----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.12.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(JUCE)

# -----------------------------------------------------------------------------
# Plugin Target
# -----------------------------------------------------------------------------
juce_add_plugin(AntigravPlateReverb
    COMPANY_NAME "Antigravity"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    # Formats to build
    FORMATS VST3 Standalone

    PRODUCT_NAME "Antigrav Plate Reverb"
    PLUGIN_NAME "Antigrav Plate Reverb"
    VERSION "0.1.0"
    description "A lush stereo algorithmic plate reverb."
)

juce_generate_juce_header(AntigravPlateReverb)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
target_sources(AntigravPlateReverb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/DelayLine.h
        Source/DSP/AllpassFilter.h
        Source/DSP/LFO.h
        Source/DSP/EarlyReflections.h
        Source/DSP/LateReverb.h
        Source/DSP/Filters.h
        Source/UI/LookAndFeel.h
)

# -----------------------------------------------------------------------------
# Link Libraries
# -----------------------------------------------------------------------------
target_link_libraries(AntigravPlateReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(AntigravPlateReverb
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
juce_add_console_app(AntigravPlateReverb_Tests
    PRODUCT_NAME "Antigrav Plate Reverb Tests"
    VERSION "0.1.0"
)

juce_generate_juce_header(AntigravPlateReverb_Tests)

target_sources(AntigravPlateReverb_Tests
    PRIVATE
        Tests/TestRunner.cpp
        Tests/DelayLineTests.cpp
        Tests/DSPTests.cpp
        Tests/EngineTests.cpp
)

target_link_libraries(AntigravPlateReverb_Tests
    PRIVATE
        juce::juce_core
        juce::juce_events
        juce::juce_data_structures
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(AntigravPlateReverb_Tests
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
