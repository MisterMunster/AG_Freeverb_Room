#pragma once

#include "AllpassFilter.h"
#include "DelayLine.h"
#include "Filters.h"
#include "LFO.h"
#include <JuceHeader.h>
#include <array>

namespace DSP {
/**
 * @brief Late Reverb engine using 8-channel FDN.
 */
class LateReverb {
public:
  LateReverb() = default;

  void prepare(double sr) {
    this->sampleRate = sr;

    // FDN Delays: Prime numbers around 30-100ms
    // FDN Delays: Prime numbers around 30-100ms
    // Moved initialization of values to setParameters or member init to allow
    // scaling. We need base values to scale from.

    for (int i = 0; i < 8; ++i) {
      delayLines[i].prepare(sampleRate, 1000.0); // Allow larger size (up to 1s)

      lfos[i].prepare(sampleRate);
      lfos[i].setFrequency(0.5f +
                           (float)i * 0.05f); // Spread LFO rates slightly
      lfos[i].setDepth(0.0f);

      hiCutFilters[i].reset();
      loCutFilters[i].reset();
    }

    // Input diffusers
    for (auto &apf : inputDiffusersL)
      apf.prepare(sampleRate, 13.0);
    for (auto &apf : inputDiffusersR)
      apf.prepare(sampleRate, 19.0); // Decorrelated

    // Initialize internal allpasses (Density)
    const float apfDelays[8] = {8.3f,  13.1f, 16.7f,  21.1f,
                                26.9f, 31.3f, 11.23f, 19.7f};
    for (int i = 0; i < 8; ++i) {
      internalAllpasses[i].prepare(sampleRate, apfDelays[i]);
      internalAllpasses[i].setFeedback(0.5f);
    }

    reset();
  }

  void reset() {
    for (auto &d : delayLines)
      d.reset();
    std::fill(std::begin(outputs), std::end(outputs), 0.0f);
  }

  void setParameters(float decayTimeS, float modDepth, float modRate,
                     float hiCut, float loCut, float sizeScaler,
                     float diffusion) {
    // Calculate feedback gain from decay time (T60)
    float avgDelayMs =
        60.0f * sizeScaler; // Approx average delay scales with size
    feedbackGain = std::pow(0.001f, (avgDelayMs / 1000.0f) / decayTimeS);

    // Modulation & Filters
    for (int i = 0; i < 8; ++i) {
      lfos[i].setFrequency(modRate * (0.9f + 0.02f * i)); // Slight variation
      lfos[i].setDepth(modDepth * 3.0f);                  // Max 3 samples shift

      hiCutFilters[i].setCoefficients(sampleRate, hiCut,
                                      OnePoleFilter::Type::LowPass);
      loCutFilters[i].setCoefficients(sampleRate, loCut,
                                      OnePoleFilter::Type::HighPass);

      // Update nominal delay times dynamically based on size
      nominalDelayTimes[i] = baseDelays[i] * sizeScaler;
    }

    // Scale density allpasses
    const float apfDelays[8] = {8.3f,  13.1f, 16.7f,  21.1f,
                                26.9f, 31.3f, 11.23f, 19.7f};
    for (int i = 0; i < 8; ++i) {
      // Scale AP delays, but clamp minimal to avoid ringing
      float newDelay = apfDelays[i] * sizeScaler;
      if (newDelay < 1.0f)
        newDelay = 1.0f;
      internalAllpasses[i].prepare(sampleRate, newDelay);

      // Internal density allpasses usually have fixed high feedback (e.g. 0.5
      // or 0.7) But we can modulate them slightly with diffusion too? For now,
      // let's keep them fixed for density, and use 'diffusion' param for Input
      // Diffusers.
      internalAllpasses[i].setFeedback(
          0.5f + (diffusion * 0.2f)); // Slight density control
    }

    // Update Input Diffusers with Diffusion parameter
    // Range roughly 0.0 to 0.7? Too high rings.
    float diffGain = diffusion * 0.65f;
    for (auto &apf : inputDiffusersL)
      apf.setFeedback(diffGain);
    for (auto &apf : inputDiffusersR)
      apf.setFeedback(diffGain);
  }

  void processBlock(juce::AudioBuffer<float> &buffer) {
    auto *left = buffer.getWritePointer(0);
    auto *right = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    for (int n = 0; n < numSamples; ++n) {
      float inL = left[n];
      float inR = right[n];

      // Diffuse Input
      // ... (simplified loop for brevity)

      // FDN mixing (Hadamard-like or Householder)
      // New inputs to delays = Input + Matrix * DelayedValues
      // Simple Householder: y = x - 2/N * sum(x)

      // Read from all delays first
      float delayOuts[8];
      for (int i = 0; i < 8; ++i) {
        float mod = lfos[i].process();
        delayOuts[i] = delayLines[i].read(nominalDelayTimes[i] + mod);
        outputs[i] = delayOuts[i]; // Store filter state if needed
      }

      // Matrix (Householder)
      // input vector 'x' is delayOuts scaled by feedback + new input

      float sum = 0.0f;
      for (int i = 0; i < 8; ++i)
        sum += delayOuts[i];
      sum *= (2.0f / 8.0f);

      float feedbackOuts[8];
      for (int i = 0; i < 8; ++i) {
        float matrixOut = delayOuts[i] - sum; // Householder
        // Add input (distribute to channels)
        // L -> 0,1,2,3. R -> 4,5,6,7
        float injection = (i < 4) ? inL : inR;
        feedbackOuts[i] = injection + matrixOut * feedbackGain;
      }

      // Filter and Push
      for (int i = 0; i < 8; ++i) {
        float processed = feedbackOuts[i];

        // Increase density
        processed = internalAllpasses[i].process(processed);

        processed = hiCutFilters[i].process(processed);
        processed = loCutFilters[i].process(processed);

        delayLines[i].push(processed);
      }

      // Output Mix
      // Sum stereo groups
      float outL = delayOuts[0] + delayOuts[1] + delayOuts[2] + delayOuts[3];
      float outR = delayOuts[4] + delayOuts[5] + delayOuts[6] + delayOuts[7];

      left[n] = outL * 0.3f; // Scaling
      right[n] = outR * 0.3f;
    }
  }

private:
  double sampleRate = 44100.0;

  std::array<DelayLine, 8> delayLines;
  std::array<LFO, 8> lfos;
  float nominalDelayTimes[8];

  const float baseDelays[8] = {29.1f, 37.3f, 44.9f, 53.7f,
                               61.3f, 79.1f, 88.7f, 97.1f};

  float outputs[8];

  std::array<AllpassFilter, 2> inputDiffusersL;
  std::array<AllpassFilter, 2> inputDiffusersR;

  // Internal allpasses for high density (smearing transients over time)
  std::array<AllpassFilter, 8> internalAllpasses;

  std::array<OnePoleFilter, 8> hiCutFilters;
  std::array<OnePoleFilter, 8> loCutFilters;

  float feedbackGain = 0.5f;
};
} // namespace DSP
