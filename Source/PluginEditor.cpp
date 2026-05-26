#include "PluginEditor.h"

namespace {
constexpr auto tiltParameterId = "tiltDb";
constexpr auto burnParameterId = "burn";
constexpr auto pulseDepthParameterId = "pulseDepth";
constexpr auto pulseRateParameterId = "pulseRate";
constexpr auto pulseMixParameterId = "pulseMix";
constexpr auto filterCutoffParameterId = "filterCutoff";
constexpr auto filterResonanceParameterId = "filterResonance";
constexpr auto pulseFilterDepthParameterId = "pulseFilterDepth";
constexpr auto delayFeedbackParameterId = "delayFeedback";
constexpr auto delayMixParameterId = "delayMix";
constexpr auto reverbMixParameterId = "reverbMix";

void configureSlider(juce::Slider& slider) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 96, 24);
}

void configureLabel(juce::Label& label, const juce::String& text) {
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}
} // namespace

FlorescenceAudioProcessorEditor::FlorescenceAudioProcessorEditor(
    FlorescenceAudioProcessor& ownerProcessor)
    : AudioProcessorEditor(&ownerProcessor), audioProcessor(ownerProcessor) {
    configureSlider(tiltSlider);
    configureSlider(burnSlider);
    configureSlider(pulseDepthSlider);
    configureSlider(pulseRateSlider);
    configureSlider(pulseMixSlider);
    configureSlider(filterCutoffSlider);
    configureSlider(filterResonanceSlider);
    configureSlider(pulseFilterDepthSlider);
    configureSlider(delayFeedbackSlider);
    configureSlider(delayMixSlider);
    configureSlider(reverbMixSlider);
    configureLabel(tiltLabel, "Tilt");
    configureLabel(burnLabel, "Burn");
    configureLabel(pulseDepthLabel, "Pulse Depth");
    configureLabel(pulseRateLabel, "Pulse Rate");
    configureLabel(pulseMixLabel, "Pulse Mix");
    configureLabel(filterCutoffLabel, "Filter Cutoff");
    configureLabel(filterResonanceLabel, "Filter Res");
    configureLabel(pulseFilterDepthLabel, "Pulse Filter");
    configureLabel(delayFeedbackLabel, "Delay Feedback");
    configureLabel(delayMixLabel, "Delay Mix");
    configureLabel(reverbMixLabel, "Reverb Mix");

    addAndMakeVisible(tiltSlider);
    addAndMakeVisible(burnSlider);
    addAndMakeVisible(pulseDepthSlider);
    addAndMakeVisible(pulseRateSlider);
    addAndMakeVisible(pulseMixSlider);
    addAndMakeVisible(filterCutoffSlider);
    addAndMakeVisible(filterResonanceSlider);
    addAndMakeVisible(pulseFilterDepthSlider);
    addAndMakeVisible(delayFeedbackSlider);
    addAndMakeVisible(delayMixSlider);
    addAndMakeVisible(reverbMixSlider);
    addAndMakeVisible(tiltLabel);
    addAndMakeVisible(burnLabel);
    addAndMakeVisible(pulseDepthLabel);
    addAndMakeVisible(pulseRateLabel);
    addAndMakeVisible(pulseMixLabel);
    addAndMakeVisible(filterCutoffLabel);
    addAndMakeVisible(filterResonanceLabel);
    addAndMakeVisible(pulseFilterDepthLabel);
    addAndMakeVisible(delayFeedbackLabel);
    addAndMakeVisible(delayMixLabel);
    addAndMakeVisible(reverbMixLabel);

    auto& parameters = audioProcessor.getParameters();
    tiltAttachment = std::make_unique<SliderAttachment>(parameters, tiltParameterId, tiltSlider);
    burnAttachment = std::make_unique<SliderAttachment>(parameters, burnParameterId, burnSlider);
    pulseDepthAttachment =
        std::make_unique<SliderAttachment>(parameters, pulseDepthParameterId, pulseDepthSlider);
    pulseRateAttachment =
        std::make_unique<SliderAttachment>(parameters, pulseRateParameterId, pulseRateSlider);
    pulseMixAttachment =
        std::make_unique<SliderAttachment>(parameters, pulseMixParameterId, pulseMixSlider);
    filterCutoffAttachment =
        std::make_unique<SliderAttachment>(parameters, filterCutoffParameterId, filterCutoffSlider);
    filterResonanceAttachment = std::make_unique<SliderAttachment>(
        parameters, filterResonanceParameterId, filterResonanceSlider);
    pulseFilterDepthAttachment = std::make_unique<SliderAttachment>(
        parameters, pulseFilterDepthParameterId, pulseFilterDepthSlider);
    delayFeedbackAttachment = std::make_unique<SliderAttachment>(
        parameters, delayFeedbackParameterId, delayFeedbackSlider);
    delayMixAttachment =
        std::make_unique<SliderAttachment>(parameters, delayMixParameterId, delayMixSlider);
    reverbMixAttachment =
        std::make_unique<SliderAttachment>(parameters, reverbMixParameterId, reverbMixSlider);

    setSize(1400, 260);
}

void FlorescenceAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colours::black);
    graphics.setColour(juce::Colours::white);
    graphics.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    graphics.drawFittedText("Florescence", getLocalBounds().removeFromTop(48),
                            juce::Justification::centred, 1);
}

void FlorescenceAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds().reduced(24);
    bounds.removeFromTop(40);

    auto columnWidth = bounds.getWidth() / 11;
    auto tiltColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto burnColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto pulseDepthColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto pulseRateColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto pulseMixColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto filterCutoffColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto filterResonanceColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto pulseFilterDepthColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto delayFeedbackColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto delayMixColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto reverbMixColumn = bounds.reduced(8);

    tiltLabel.setBounds(tiltColumn.removeFromTop(24));
    tiltSlider.setBounds(tiltColumn);
    burnLabel.setBounds(burnColumn.removeFromTop(24));
    burnSlider.setBounds(burnColumn);
    pulseDepthLabel.setBounds(pulseDepthColumn.removeFromTop(24));
    pulseDepthSlider.setBounds(pulseDepthColumn);
    pulseRateLabel.setBounds(pulseRateColumn.removeFromTop(24));
    pulseRateSlider.setBounds(pulseRateColumn);
    pulseMixLabel.setBounds(pulseMixColumn.removeFromTop(24));
    pulseMixSlider.setBounds(pulseMixColumn);
    filterCutoffLabel.setBounds(filterCutoffColumn.removeFromTop(24));
    filterCutoffSlider.setBounds(filterCutoffColumn);
    filterResonanceLabel.setBounds(filterResonanceColumn.removeFromTop(24));
    filterResonanceSlider.setBounds(filterResonanceColumn);
    pulseFilterDepthLabel.setBounds(pulseFilterDepthColumn.removeFromTop(24));
    pulseFilterDepthSlider.setBounds(pulseFilterDepthColumn);
    delayFeedbackLabel.setBounds(delayFeedbackColumn.removeFromTop(24));
    delayFeedbackSlider.setBounds(delayFeedbackColumn);
    delayMixLabel.setBounds(delayMixColumn.removeFromTop(24));
    delayMixSlider.setBounds(delayMixColumn);
    reverbMixLabel.setBounds(reverbMixColumn.removeFromTop(24));
    reverbMixSlider.setBounds(reverbMixColumn);
}
