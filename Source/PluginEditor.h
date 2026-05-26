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
    juce::Slider burnSlider;
    juce::Slider pulseDepthSlider;
    juce::Slider pulseRateSlider;
    juce::Slider pulseMixSlider;
    juce::Slider filterCutoffSlider;
    juce::Slider filterResonanceSlider;
    juce::Slider pulseFilterDepthSlider;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayMixSlider;
    juce::Label tiltLabel;
    juce::Label burnLabel;
    juce::Label pulseDepthLabel;
    juce::Label pulseRateLabel;
    juce::Label pulseMixLabel;
    juce::Label filterCutoffLabel;
    juce::Label filterResonanceLabel;
    juce::Label pulseFilterDepthLabel;
    juce::Label delayFeedbackLabel;
    juce::Label delayMixLabel;

    std::unique_ptr<SliderAttachment> tiltAttachment;
    std::unique_ptr<SliderAttachment> burnAttachment;
    std::unique_ptr<SliderAttachment> pulseDepthAttachment;
    std::unique_ptr<SliderAttachment> pulseRateAttachment;
    std::unique_ptr<SliderAttachment> pulseMixAttachment;
    std::unique_ptr<SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<SliderAttachment> pulseFilterDepthAttachment;
    std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<SliderAttachment> delayMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlorescenceAudioProcessorEditor)
};
