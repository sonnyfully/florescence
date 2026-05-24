#include "PluginEditor.h"

namespace {
constexpr auto gainParameterId = "inputGain";
constexpr auto cutoffParameterId = "cutoffHz";

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
    configureSlider(gainSlider);
    configureSlider(cutoffSlider);
    configureLabel(gainLabel, "Gain");
    configureLabel(cutoffLabel, "Low Pass");

    addAndMakeVisible(gainSlider);
    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(cutoffLabel);

    auto& parameters = audioProcessor.getParameters();
    gainAttachment = std::make_unique<SliderAttachment>(parameters, gainParameterId, gainSlider);
    cutoffAttachment =
        std::make_unique<SliderAttachment>(parameters, cutoffParameterId, cutoffSlider);

    setSize(360, 220);
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

    auto left = bounds.removeFromLeft(bounds.getWidth() / 2).reduced(12);
    auto right = bounds.reduced(12);

    gainLabel.setBounds(left.removeFromTop(24));
    gainSlider.setBounds(left);
    cutoffLabel.setBounds(right.removeFromTop(24));
    cutoffSlider.setBounds(right);
}
