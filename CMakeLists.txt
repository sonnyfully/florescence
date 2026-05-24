cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version supported by the plugin")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build a Universal Binary for macOS hosts")

project(Florescence VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CHARACTERFX_BUILD_PLUGIN "Build the JUCE plugin targets" ON)
option(CHARACTERFX_BUILD_TESTS "Build the Catch2 test suite" ON)

if(CHARACTERFX_BUILD_PLUGIN)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
        message(FATAL_ERROR "JUCE is required at external/JUCE. Add it before configuring Stage 1.")
    endif()

    add_subdirectory(external/JUCE)
endif()

if(CHARACTERFX_BUILD_TESTS)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/Catch2/CMakeLists.txt")
        message(FATAL_ERROR "Catch2 is required at external/Catch2. Add it before configuring Stage 1 tests.")
    endif()

    add_subdirectory(external/Catch2)
endif()

if(CHARACTERFX_BUILD_PLUGIN)
    juce_add_plugin(CharacterFX
        COMPANY_NAME "Solace"
        BUNDLE_ID "com.solace.florescence"
        PLUGIN_MANUFACTURER_CODE Slce
        PLUGIN_CODE flrs
        FORMATS AU VST3 Standalone
        PRODUCT_NAME "Florescence"
        DESCRIPTION "Stereo character processor"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Fx
        AU_MAIN_TYPE kAudioUnitType_Effect)

    target_sources(CharacterFX
        PRIVATE
            Source/DSP/ThrowawayOnePoleLowPass.cpp
            Source/PluginEditor.cpp
            Source/PluginProcessor.cpp)

    target_compile_definitions(CharacterFX
        PUBLIC
            JUCE_REPORT_APP_USAGE=0
            JUCE_USE_CURL=0
            JUCE_WEB_BROWSER=0
            JUCE_VST3_CAN_REPLACE_VST2=0)

    target_link_libraries(CharacterFX
        PRIVATE
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_gui_basics
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
endif()

if(CHARACTERFX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()
