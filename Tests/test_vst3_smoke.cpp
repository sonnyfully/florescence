#include <catch2/catch_test_macros.hpp>

#include <juce_audio_processors_headless/juce_audio_processors_headless.h>

TEST_CASE("built VST3 loads and exposes north-star-aligned stage 3 parameters") {
    juce::VST3PluginFormatHeadless format;
    juce::OwnedArray<juce::PluginDescription> descriptions;

    format.findAllTypesForFile(descriptions, CHARACTERFX_VST3_PATH);

    REQUIRE(descriptions.size() == 1);

    const auto& description = *descriptions[0];
    REQUIRE(description.name == "Florescence");
    REQUIRE(description.manufacturerName == "Solace");
    REQUIRE(description.pluginFormatName == "VST3");

    juce::String error;
    auto instance = format.createInstanceFromDescription(description, 48000.0, 512, error);

    INFO(error);
    REQUIRE(instance != nullptr);
    REQUIRE(instance->getName() == "Florescence");
    REQUIRE(instance->getTotalNumInputChannels() == 2);
    REQUIRE(instance->getTotalNumOutputChannels() == 2);

    const auto& parameters = instance->getParameters();
    REQUIRE(parameters.size() >= 2);

    juce::StringArray parameterNames;
    for (auto* parameter : parameters)
        parameterNames.add(parameter->getName(64));

    REQUIRE(parameterNames.contains("Output"));
    REQUIRE(parameterNames.contains("Tilt"));
    REQUIRE(parameterNames.contains("Burn"));
    REQUIRE(parameterNames.contains("Pulse Depth"));
    REQUIRE(parameterNames.contains("Pulse Rate"));
    REQUIRE(parameterNames.contains("Pulse Mix"));
    REQUIRE(parameterNames.contains("Filter Cutoff"));
    REQUIRE(parameterNames.contains("Filter Resonance"));
    REQUIRE(parameterNames.contains("Pulse Filter"));
}
