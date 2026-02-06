cmake_minimum_required(VERSION 3.22)

project(AntigravReverb VERSION 0.1.0 LANGUAGES C CXX)

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
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master # Or specify a specific tag/hash for stability, e.g., 8.0.0
)
FetchContent_MakeAvailable(JUCE)

# -----------------------------------------------------------------------------
# Plugin Target
# -----------------------------------------------------------------------------
juce_add_plugin(AntigravReverb
    COMPANY_NAME "Antigravity"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    # Formats to build
    FORMATS VST3 Standalone

    PRODUCT_NAME "Antigrav Reverb"
    PLUGIN_NAME "Antigrav Reverb"
    VERSION "0.1.0"
    description "A true stereo algorithmic reverb."
)

juce_generate_juce_header(AntigravReverb)

# -----------------------------------------------------------------------------
# Binary Data
# -----------------------------------------------------------------------------
juce_add_binary_data(AntigravReverb_Data
    SOURCES 
        AGreverb.jpg
)


# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
target_sources(AntigravReverb
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
        Source/DSP/LateReverb.h
        Source/DSP/Filters.h
        Source/UI/LookAndFeel.h
)

# -----------------------------------------------------------------------------
# Link Libraries
# -----------------------------------------------------------------------------
target_link_libraries(AntigravReverb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        AntigravReverb_Data
)

target_compile_definitions(AntigravReverb
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# -----------------------------------------------------------------------------
# Unit Tests
# -----------------------------------------------------------------------------
juce_add_console_app(AntigravReverb_Tests
    PRODUCT_NAME "Antigrav Reverb Tests"
    VERSION "0.1.0"
)

juce_generate_juce_header(AntigravReverb_Tests)

target_sources(AntigravReverb_Tests
    PRIVATE
        Tests/TestRunner.cpp
        Tests/DelayLineTests.cpp
        Tests/DSPTests.cpp
        Tests/EngineTests.cpp
        # Filter/DSP sources will be added here as we create them
        # For now, we might need to expose DSP code as a separate static lib 
        # to share between Plugin and Tests to avoid including .cpp files directly.
        # But for simplicity, we can just include the source files here for now.
)

target_link_libraries(AntigravReverb_Tests
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
