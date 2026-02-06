#pragma once

#include "AllpassFilter.h"
#include "DelayLine.h"
#include "Filters.h"
#include <JuceHeader.h>
#include <array>

namespace DSP {
/**
 * @brief Early Reflections engine.
 * Uses input diffusion (series allpasses) followed by a multi-tap delay
 * structure.
 */
class EarlyReflections {
public:
  EarlyReflections() = default;

  void prepare(double sr) {
    this->sampleRate = sr;

    // Initialize diffusers
    for (auto &apf : diffusersL)
      apf.prepare(sr, 10.0); // Arbitrary max
    for (auto &apf : diffusersR)
      apf.prepare(sr, 10.0);

    // Initialize main delays
    delayL.prepare(sr, 500.0); // Max 500ms size
    delayR.prepare(sr, 500.0);
    directDelayL.prepare(sr, 500.0);
    directDelayR.prepare(sr, 500.0);

    reset();
  }

  void reset() {
    for (auto &apf : diffusersL)
      apf.setFeedback(0.0f);
    for (auto &apf : diffusersR)
      apf.setFeedback(0.0f);
    delayL.reset();
    delayR.reset();
    directDelayL.reset();
    directDelayR.reset();
    hiCutL.reset();
    hiCutR.reset();
    loCutL.reset();
    loCutR.reset();
  }

  void setParameters(float sizeMs, float cross, float diffusionAmt,
                     float hiCutHz, float loCutHz) {
    currentSizeMs = sizeMs;
    currentCross = cross;

    // Update filters
    hiCutL.setCoefficients(sampleRate, hiCutHz, OnePoleFilter::Type::LowPass);
    hiCutR.setCoefficients(sampleRate, hiCutHz, OnePoleFilter::Type::LowPass);
    loCutL.setCoefficients(sampleRate, loCutHz, OnePoleFilter::Type::HighPass);
    loCutR.setCoefficients(sampleRate, loCutHz, OnePoleFilter::Type::HighPass);

    // Update diffusion coefficients
    float diff = diffusionAmt * 0.6f; // Max correlation
    for (auto &apf : diffusersL)
      apf.setFeedback(diff);
    for (auto &apf : diffusersR)
      apf.setFeedback(diff);

    // Update diffuser delay times (scaled by size generally, or fixed for
    // character) Sticking to fixed decent prime numbers for "smearing" or
    // scaled. Let's scale them slightly with size to keep density consistent
    // relative to room. Values: 4.7, 8.3, 11.1 ms etc.
    static const float baseDelays[] = {4.3f, 7.1f, 13.7f};
    for (size_t i = 0; i < 3; ++i) {
      // Keeping diffusers relatively short/fixed usually better for ER
      diffusersL[i].prepare(sampleRate, baseDelays[i]);
      diffusersR[i].prepare(sampleRate,
                            baseDelays[i] + 2.3f); // Decorrelate L/R
    }
  }

  // Processing stereo block
  void processBlock(juce::AudioBuffer<float> &buffer) {
    auto *left = buffer.getWritePointer(0);
    auto *right = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i) {
      float inL = left[i];
      float inR = right[i];

      // 1. Crossfeed Input
      float mixedL =
          inL * (1.0f - currentCross * 0.5f) + inR * (currentCross * 0.5f);
      float mixedR =
          inR * (1.0f - currentCross * 0.5f) + inL * (currentCross * 0.5f);

      // 2. Direct Discrete Taps (Clean, sharp reflections)
      directDelayL.push(mixedL);
      directDelayR.push(mixedR);

      float outL = 0.0f;
      float outR = 0.0f;

      // Distinct early echoes (e.g. floor/wall reflections)
      // ITDG area ~10-20ms
      outL += directDelayL.read(currentSizeMs * 0.08f) * 0.4f;
      outL += directDelayL.read(currentSizeMs * 0.19f) * 0.3f;

      outR += directDelayR.read(currentSizeMs * 0.09f) * 0.4f;
      outR += directDelayR.read(currentSizeMs * 0.21f) * 0.3f;

      // 3. Diffusion (Smearing for body of ER)
      float diffL = mixedL;
      float diffR = mixedR;
      for (auto &apf : diffusersL)
        diffL = apf.process(diffL);
      for (auto &apf : diffusersR)
        diffR = apf.process(diffR);

      // 4. Diffused Taps
      delayL.push(diffL);
      delayR.push(diffR);

      // Higher density, later taps
      outL += delayL.read(currentSizeMs * 0.35f) * 0.3f;
      outL += delayL.read(currentSizeMs * 0.55f) * 0.25f;
      outL += delayR.read(currentSizeMs * 0.72f) * 0.2f; // Cross
      outL += delayL.read(currentSizeMs * 0.93f) * 0.15f;

      outR += delayR.read(currentSizeMs * 0.38f) * 0.3f;
      outR += delayR.read(currentSizeMs * 0.58f) * 0.25f;
      outR += delayL.read(currentSizeMs * 0.75f) * 0.2f; // Cross
      outR += delayR.read(currentSizeMs * 0.96f) * 0.15f;

      // 5. Output Filtering (Absorption)
      outL = hiCutL.process(outL);
      outL = loCutL.process(outL);

      outR = hiCutR.process(outR);
      outR = loCutR.process(outR);

      left[i] = outL;
      right[i] = outR;
    }
  }

private:
  double sampleRate = 44100.0;

  std::array<AllpassFilter, 3> diffusersL;
  std::array<AllpassFilter, 3> diffusersR;

  DelayLine delayL, delayR;             // Diffused path
  DelayLine directDelayL, directDelayR; // Discrete path

  // Output filters for wall absorption simulation
  OnePoleFilter hiCutL, hiCutR;
  OnePoleFilter loCutL, loCutR;

  float currentSizeMs = 300.0f;
  float currentCross = 0.1f;
};
} // namespace DSP
