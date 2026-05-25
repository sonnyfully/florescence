#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

class FlorescenceAudioProcessorEditor final : public juce::AudioProcessorEditor {
  public:
    explicit FlorescenceAudioProcessorEditor(FlorescenceAudioProcessor& processor);
    ~FlorescenceAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

  private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    FlorescenceAudioProcessor& audioProcessor;

    juce::Slider tiltSlider;
    juce::Slider saturationSlider;
    juce::Slider chorusDepthSlider;
    juce::Slider chorusRateSlider;
    juce::Slider chorusMixSlider;
    juce::Slider filterCutoffSlider;
    juce::Slider filterResonanceSlider;
    juce::Slider filterEnvelopeDepthSlider;
    juce::Label tiltLabel;
    juce::Label saturationLabel;
    juce::Label chorusDepthLabel;
    juce::Label chorusRateLabel;
    juce::Label chorusMixLabel;
    juce::Label filterCutoffLabel;
    juce::Label filterResonanceLabel;
    juce::Label filterEnvelopeDepthLabel;

    std::unique_ptr<SliderAttachment> tiltAttachment;
    std::unique_ptr<SliderAttachment> saturationAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<SliderAttachment> filterEnvelopeDepthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlorescenceAudioProcessorEditor)
};
