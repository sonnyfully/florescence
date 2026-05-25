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
    juce::Label tiltLabel;
    juce::Label saturationLabel;

    std::unique_ptr<SliderAttachment> tiltAttachment;
    std::unique_ptr<SliderAttachment> saturationAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlorescenceAudioProcessorEditor)
};
