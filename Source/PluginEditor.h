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

    juce::Slider gainSlider;
    juce::Slider tiltSlider;
    juce::Label gainLabel;
    juce::Label tiltLabel;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> tiltAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlorescenceAudioProcessorEditor)
};
