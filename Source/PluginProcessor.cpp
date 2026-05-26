#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace {
constexpr auto outputTrimParameterId = "outputTrim";
constexpr auto tiltParameterId = "tiltDb";
constexpr auto burnParameterId = "burn";
constexpr auto pulseDepthParameterId = "pulseDepth";
constexpr auto pulseRateParameterId = "pulseRate";
constexpr auto pulseMixParameterId = "pulseMix";
constexpr auto filterCutoffParameterId = "filterCutoff";
constexpr auto filterResonanceParameterId = "filterResonance";
constexpr auto pulseFilterDepthParameterId = "pulseFilterDepth";
constexpr auto delayDivisionParameterId = "delayDivision";
constexpr auto delayFeedbackParameterId = "delayFeedback";
constexpr auto delayMixParameterId = "delayMix";
constexpr auto delayTopologyParameterId = "delayTopology";
constexpr auto reverbMixParameterId = "reverbMix";

juce::StringArray getDelayDivisionNames() {
    juce::StringArray names;

    for (auto index = 0; index < Delay::getSyncDivisionCount(); ++index)
        names.add(Delay::getSyncDivisionName(Delay::syncDivisionFromIndex(index)));

    return names;
}
} // namespace

FlorescenceAudioProcessor::FlorescenceAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout()) {
    auto tiltModule = std::make_unique<TiltEQ>();
    tiltEq = tiltModule.get();
    fxChain.push_back(std::move(tiltModule));

    auto saturationModule = std::make_unique<Saturation>();
    saturation = saturationModule.get();
    fxChain.push_back(std::move(saturationModule));

    auto chorusModule = std::make_unique<Chorus>();
    chorus = chorusModule.get();
    fxChain.push_back(std::move(chorusModule));

    auto filterModule = std::make_unique<Filter>();
    filter = filterModule.get();
    fxChain.push_back(std::move(filterModule));

    auto delayModule = std::make_unique<Delay>();
    delay = delayModule.get();
    fxChain.push_back(std::move(delayModule));

    auto convReverbModule = std::make_unique<ConvReverb>();
    convReverb = convReverbModule.get();
    fxChain.push_back(std::move(convReverbModule));
}

juce::AudioProcessorValueTreeState::ParameterLayout
FlorescenceAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> layout;

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{outputTrimParameterId, 1}, "Output",
        juce::NormalisableRange<float>{-24.0f, 12.0f, 0.01f}, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{tiltParameterId, 1}, "Tilt",
        juce::NormalisableRange<float>{dspconfig::tiltEqMinDb, dspconfig::tiltEqMaxDb, 0.01f}, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{burnParameterId, 1}, "Burn",
        juce::NormalisableRange<float>{saturationconfig::driveMin, saturationconfig::driveMax,
                                       0.001f},
        0.0f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{pulseDepthParameterId, 1}, "Pulse Depth",
        juce::NormalisableRange<float>{chorusconfig::depthMin, chorusconfig::depthMax, 0.001f},
        0.5f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{pulseRateParameterId, 1}, "Pulse Rate",
        juce::NormalisableRange<float>{chorusconfig::rateMinHz, chorusconfig::rateMaxHz, 0.001f,
                                       0.5f},
        0.8f, juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{pulseMixParameterId, 1}, "Pulse Mix",
        juce::NormalisableRange<float>{chorusconfig::mixMin, chorusconfig::mixMax, 0.001f}, 0.0f));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{filterCutoffParameterId, 1}, "Filter Cutoff",
        juce::NormalisableRange<float>{filterconfig::cutoffMinHz, filterconfig::cutoffMaxHz, 0.001f,
                                       0.35f},
        filterconfig::cutoffDefaultHz, juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{filterResonanceParameterId, 1}, "Filter Resonance",
        juce::NormalisableRange<float>{filterconfig::resonanceMin, filterconfig::resonanceMax,
                                       0.001f},
        filterconfig::resonanceDefault));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{pulseFilterDepthParameterId, 1}, "Pulse Filter",
        juce::NormalisableRange<float>{filterconfig::envelopeDepthMinOctaves,
                                       filterconfig::envelopeDepthMaxOctaves, 0.001f},
        filterconfig::envelopeDefaultDepthOctaves,
        juce::AudioParameterFloatAttributes().withLabel("oct")));

    layout.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{delayDivisionParameterId, 1}, "Delay Division", getDelayDivisionNames(),
        Delay::syncDivisionToIndex(Delay::getDefaultSyncDivision())));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{delayFeedbackParameterId, 1}, "Delay Feedback",
        juce::NormalisableRange<float>{delayconfig::feedbackMin, delayconfig::feedbackMax, 0.001f},
        delayconfig::feedbackDefault));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{delayMixParameterId, 1}, "Delay Mix",
        juce::NormalisableRange<float>{delayconfig::mixMin, delayconfig::mixMax, 0.001f},
        delayconfig::mixDefault));

    layout.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{delayTopologyParameterId, 1}, "Delay Topology",
        juce::StringArray{"Stereo", "Ping Pong"}, static_cast<int>(Delay::Topology::Stereo)));

    layout.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{reverbMixParameterId, 1}, "Reverb Mix",
        juce::NormalisableRange<float>{convreverbconfig::wetMixMin, convreverbconfig::wetMixMax,
                                       0.001f},
        convreverbconfig::wetMixDefault));

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

void FlorescenceAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock) {
    const juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock),
                                      static_cast<juce::uint32>(getTotalNumOutputChannels())};

    auto totalLatencySamples = 0;
    for (auto& module : fxChain) {
        module->prepare(spec);
        totalLatencySamples += module->getLatencySamples();
    }

    setLatencySamples(totalLatencySamples);
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

    const auto outputTrimDb = parameters.getRawParameterValue(outputTrimParameterId)->load();
    const auto tiltDb = parameters.getRawParameterValue(tiltParameterId)->load();
    const auto burn = parameters.getRawParameterValue(burnParameterId)->load();
    const auto pulseDepth = parameters.getRawParameterValue(pulseDepthParameterId)->load();
    const auto pulseRateHz = parameters.getRawParameterValue(pulseRateParameterId)->load();
    const auto pulseMix = parameters.getRawParameterValue(pulseMixParameterId)->load();
    const auto filterCutoffHz = parameters.getRawParameterValue(filterCutoffParameterId)->load();
    const auto filterResonance =
        parameters.getRawParameterValue(filterResonanceParameterId)->load();
    const auto pulseFilterDepth =
        parameters.getRawParameterValue(pulseFilterDepthParameterId)->load();
    const auto delayDivisionIndex =
        static_cast<int>(parameters.getRawParameterValue(delayDivisionParameterId)->load());
    const auto delayFeedback = parameters.getRawParameterValue(delayFeedbackParameterId)->load();
    const auto delayMix = parameters.getRawParameterValue(delayMixParameterId)->load();
    const auto delayTopologyIndex =
        static_cast<int>(parameters.getRawParameterValue(delayTopologyParameterId)->load());
    const auto reverbMix = parameters.getRawParameterValue(reverbMixParameterId)->load();
    const auto outputGain = juce::Decibels::decibelsToGain(outputTrimDb);

    if (tiltEq != nullptr)
        tiltEq->setTiltDb(tiltDb);

    if (saturation != nullptr)
        saturation->setDrive(burn);

    if (chorus != nullptr) {
        chorus->setDepth(pulseDepth);
        chorus->setRateHz(pulseRateHz);
        chorus->setMix(pulseMix);
    }

    if (filter != nullptr) {
        filter->setCutoffHz(filterCutoffHz);
        filter->setResonance(filterResonance);
        filter->setEnvelopeDepthOctaves(pulseFilterDepth);
    }

    if (delay != nullptr) {
        if (auto* playHead = getPlayHead()) {
            if (const auto position = playHead->getPosition()) {
                if (const auto bpm = position->getBpm())
                    delay->setTempoBpm(*bpm);
            }
        }

        delay->setSyncDivision(Delay::syncDivisionFromIndex(delayDivisionIndex));
        delay->setFeedback(delayFeedback);
        delay->setMix(delayMix);
        delay->setTopology(delayTopologyIndex == static_cast<int>(Delay::Topology::PingPong)
                               ? Delay::Topology::PingPong
                               : Delay::Topology::Stereo);
    }

    if (convReverb != nullptr)
        convReverb->setWetMix(reverbMix);

    buffer.applyGain(outputGain);

    auto block = juce::dsp::AudioBlock<float>(buffer);
    for (auto& module : fxChain)
        module->process(block);
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
