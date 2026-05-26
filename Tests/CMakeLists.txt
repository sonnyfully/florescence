add_executable(CharacterFX_Tests
    test_chorus.cpp
    test_convreverb.cpp
    test_delay.cpp
    test_filter.cpp
    test_tilteq.cpp
    test_jiles_atherton.cpp
    test_saturation.cpp
    ../Source/DSP/Chorus.cpp
    ../Source/DSP/ConvReverb.cpp
    ../Source/DSP/Delay.cpp
    ../Source/DSP/Filter.cpp
    ../Source/DSP/JilesAtherton.cpp
    ../Source/DSP/Saturation.cpp
    ../Source/DSP/TiltEQ.cpp)

target_include_directories(CharacterFX_Tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../Source")

target_link_libraries(CharacterFX_Tests
    PRIVATE
        Catch2::Catch2WithMain
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

include(Catch)
catch_discover_tests(CharacterFX_Tests)

if(TARGET juce::juce_audio_processors_headless AND TARGET CharacterFX_VST3)
    add_executable(CharacterFX_PluginSmokeTests
        test_vst3_smoke.cpp)

    add_dependencies(CharacterFX_PluginSmokeTests CharacterFX_VST3)

    target_compile_definitions(CharacterFX_PluginSmokeTests
        PRIVATE
            CHARACTERFX_VST3_PATH="${CMAKE_BINARY_DIR}/CharacterFX_artefacts/VST3/Florescence.vst3"
            JUCE_PLUGINHOST_VST3=1)

    target_link_libraries(CharacterFX_PluginSmokeTests
        PRIVATE
            Catch2::Catch2WithMain
            juce::juce_audio_processors_headless
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    catch_discover_tests(CharacterFX_PluginSmokeTests)
endif()
