#include "PluginEditor.h"

namespace {
constexpr auto tiltParameterId = "tiltDb";
constexpr auto saturationDriveParameterId = "saturationDrive";
constexpr auto chorusDepthParameterId = "chorusDepth";
constexpr auto chorusRateParameterId = "chorusRate";
constexpr auto chorusMixParameterId = "chorusMix";

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
    configureSlider(saturationSlider);
    configureSlider(chorusDepthSlider);
    configureSlider(chorusRateSlider);
    configureSlider(chorusMixSlider);
    configureLabel(tiltLabel, "Tilt");
    configureLabel(saturationLabel, "Saturation");
    configureLabel(chorusDepthLabel, "Chorus Depth");
    configureLabel(chorusRateLabel, "Chorus Rate");
    configureLabel(chorusMixLabel, "Chorus Mix");

    addAndMakeVisible(tiltSlider);
    addAndMakeVisible(saturationSlider);
    addAndMakeVisible(chorusDepthSlider);
    addAndMakeVisible(chorusRateSlider);
    addAndMakeVisible(chorusMixSlider);
    addAndMakeVisible(tiltLabel);
    addAndMakeVisible(saturationLabel);
    addAndMakeVisible(chorusDepthLabel);
    addAndMakeVisible(chorusRateLabel);
    addAndMakeVisible(chorusMixLabel);

    auto& parameters = audioProcessor.getParameters();
    tiltAttachment = std::make_unique<SliderAttachment>(parameters, tiltParameterId, tiltSlider);
    saturationAttachment = std::make_unique<SliderAttachment>(
        parameters, saturationDriveParameterId, saturationSlider);
    chorusDepthAttachment =
        std::make_unique<SliderAttachment>(parameters, chorusDepthParameterId, chorusDepthSlider);
    chorusRateAttachment =
        std::make_unique<SliderAttachment>(parameters, chorusRateParameterId, chorusRateSlider);
    chorusMixAttachment =
        std::make_unique<SliderAttachment>(parameters, chorusMixParameterId, chorusMixSlider);

    setSize(720, 260);
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

    auto columnWidth = bounds.getWidth() / 5;
    auto tiltColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto saturationColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusDepthColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusRateColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusMixColumn = bounds.reduced(8);

    tiltLabel.setBounds(tiltColumn.removeFromTop(24));
    tiltSlider.setBounds(tiltColumn);
    saturationLabel.setBounds(saturationColumn.removeFromTop(24));
    saturationSlider.setBounds(saturationColumn);
    chorusDepthLabel.setBounds(chorusDepthColumn.removeFromTop(24));
    chorusDepthSlider.setBounds(chorusDepthColumn);
    chorusRateLabel.setBounds(chorusRateColumn.removeFromTop(24));
    chorusRateSlider.setBounds(chorusRateColumn);
    chorusMixLabel.setBounds(chorusMixColumn.removeFromTop(24));
    chorusMixSlider.setBounds(chorusMixColumn);
}
