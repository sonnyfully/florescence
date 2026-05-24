#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace {
constexpr auto gainParameterId = "inputGain";
constexpr auto cutoffParameterId = "cutoffHz";
} // namespace

FlorescenceAudioProcessor::FlorescenceAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout
FlorescenceAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{gainParameterId, 1}, "Gain",
        juce::NormalisableRange<float>{-24.0f, 12.0f, 0.01f}, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{cutoffParameterId, 1}, "Low Pass",
        juce::NormalisableRange<float>{20.0f, 20000.0f, 0.01f, 0.35f}, 20000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    return {layout.begin(), layout.end()};
}

const juce::String FlorescenceAudioProcessor::getName() const {
    return "Florescence";
}

bool FlorescenceAudioProcessor::acceptsMidi() const {
    return false;
}

bool FlorescenceAudioProcessor::producesMidi() const {
    return false;
}

bool FlorescenceAudioProcessor::isMidiEffect() const {
    return false;
}

double FlorescenceAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int FlorescenceAudioProcessor::getNumPrograms() {
    return 1;
}

int FlorescenceAudioProcessor::getCurrentProgram() {
    return 0;
}

void FlorescenceAudioProcessor::setCurrentProgram(int) {}

const juce::String FlorescenceAudioProcessor::getProgramName(int) {
    return {};
}

void FlorescenceAudioProcessor::changeProgramName(int, const juce::String&) {}

void FlorescenceAudioProcessor::prepareToPlay(const double sampleRate, int) {
    lowPass.prepare(sampleRate, static_cast<std::size_t>(getTotalNumOutputChannels()));
}

void FlorescenceAudioProcessor::releaseResources() {}

bool FlorescenceAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    const auto& mainInput = layouts.getMainInputChannelSet();

    if (mainOutput != juce::AudioChannelSet::mono() &&
        mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return mainInput == mainOutput;
}

void FlorescenceAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const auto totalInputChannels = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();

    for (auto channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    const auto gainDb = parameters.getRawParameterValue(gainParameterId)->load();
    const auto cutoffHz = parameters.getRawParameterValue(cutoffParameterId)->load();
    const auto gain = juce::Decibels::decibelsToGain(gainDb);

    lowPass.setCutoffHz(cutoffHz);

    const auto channelCount = std::min(totalInputChannels, totalOutputChannels);
    for (auto channel = 0; channel < channelCount; ++channel) {
        auto* samples = buffer.getWritePointer(channel);
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
            samples[sample] =
                lowPass.processSample(static_cast<std::size_t>(channel), samples[sample] * gain);
    }
}

bool FlorescenceAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* FlorescenceAudioProcessor::createEditor() {
    return new FlorescenceAudioProcessorEditor(*this);
}

void FlorescenceAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    if (auto state = parameters.copyState(); auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void FlorescenceAudioProcessor::setStateInformation(const void* data, const int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new FlorescenceAudioProcessor();
}
