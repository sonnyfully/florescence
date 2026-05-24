#pragma once

#include <cstddef>
#include <vector>

class ThrowawayOnePoleLowPass {
  public:
    void prepare(double newSampleRate, std::size_t channelCount);
    void reset() noexcept;
    void setCutoffHz(float newCutoffHz) noexcept;

    [[nodiscard]] float processSample(std::size_t channel, float input) noexcept;
    [[nodiscard]] float getCoefficient() const noexcept {
        return coefficient;
    }

  private:
    double sampleRate = 48000.0;
    float cutoffHz = 1000.0f;
    float coefficient = 0.0f;
    std::vector<float> z1;

    void updateCoefficient() noexcept;
};
