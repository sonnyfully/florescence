#include "PluginEditor.h"

namespace {
constexpr auto tiltParameterId = "tiltDb";
constexpr auto saturationDriveParameterId = "saturationDrive";
constexpr auto chorusDepthParameterId = "chorusDepth";
constexpr auto chorusRateParameterId = "chorusRate";
constexpr auto chorusMixParameterId = "chorusMix";
constexpr auto filterCutoffParameterId = "filterCutoff";
constexpr auto filterResonanceParameterId = "filterResonance";
constexpr auto filterEnvelopeDepthParameterId = "filterEnvelopeDepth";

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
    configureSlider(filterCutoffSlider);
    configureSlider(filterResonanceSlider);
    configureSlider(filterEnvelopeDepthSlider);
    configureLabel(tiltLabel, "Tilt");
    configureLabel(saturationLabel, "Saturation");
    configureLabel(chorusDepthLabel, "Chorus Depth");
    configureLabel(chorusRateLabel, "Chorus Rate");
    configureLabel(chorusMixLabel, "Chorus Mix");
    configureLabel(filterCutoffLabel, "Filter Cutoff");
    configureLabel(filterResonanceLabel, "Filter Res");
    configureLabel(filterEnvelopeDepthLabel, "Filter Env");

    addAndMakeVisible(tiltSlider);
    addAndMakeVisible(saturationSlider);
    addAndMakeVisible(chorusDepthSlider);
    addAndMakeVisible(chorusRateSlider);
    addAndMakeVisible(chorusMixSlider);
    addAndMakeVisible(filterCutoffSlider);
    addAndMakeVisible(filterResonanceSlider);
    addAndMakeVisible(filterEnvelopeDepthSlider);
    addAndMakeVisible(tiltLabel);
    addAndMakeVisible(saturationLabel);
    addAndMakeVisible(chorusDepthLabel);
    addAndMakeVisible(chorusRateLabel);
    addAndMakeVisible(chorusMixLabel);
    addAndMakeVisible(filterCutoffLabel);
    addAndMakeVisible(filterResonanceLabel);
    addAndMakeVisible(filterEnvelopeDepthLabel);

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
    filterCutoffAttachment =
        std::make_unique<SliderAttachment>(parameters, filterCutoffParameterId, filterCutoffSlider);
    filterResonanceAttachment = std::make_unique<SliderAttachment>(
        parameters, filterResonanceParameterId, filterResonanceSlider);
    filterEnvelopeDepthAttachment = std::make_unique<SliderAttachment>(
        parameters, filterEnvelopeDepthParameterId, filterEnvelopeDepthSlider);

    setSize(1040, 260);
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

    auto columnWidth = bounds.getWidth() / 8;
    auto tiltColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto saturationColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusDepthColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusRateColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto chorusMixColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto filterCutoffColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto filterResonanceColumn = bounds.removeFromLeft(columnWidth).reduced(8);
    auto filterEnvelopeDepthColumn = bounds.reduced(8);

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
    filterCutoffLabel.setBounds(filterCutoffColumn.removeFromTop(24));
    filterCutoffSlider.setBounds(filterCutoffColumn);
    filterResonanceLabel.setBounds(filterResonanceColumn.removeFromTop(24));
    filterResonanceSlider.setBounds(filterResonanceColumn);
    filterEnvelopeDepthLabel.setBounds(filterEnvelopeDepthColumn.removeFromTop(24));
    filterEnvelopeDepthSlider.setBounds(filterEnvelopeDepthColumn);
}
