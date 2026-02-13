#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"
#include "AllpassFilter.h"
#include "Filters.h"
#include "LFO.h"
#include <array>

namespace DSP
{
    /**
     * @brief Plate-style Late Reverb using a 12-channel Feedback Delay Network.
     *
     * Plate reverbs differ from room reverbs in several key ways:
     * - Higher echo density (12 channels vs 8 for room)
     * - Shorter delay times (plate dimensions are compact, ~10-75ms)
     * - Brighter character (metal absorbs less HF than room surfaces)
     * - Smoother, more even decay across frequencies
     * - Stronger modulation for lush, chorused tail
     *
     * Uses a Householder mixing matrix for maximum decorrelation and
     * per-channel modulated delays to prevent metallic ringing.
     */
    class LateReverb
    {
    public:
        static constexpr int NUM_CHANNELS = 12;

        LateReverb() = default;

        void prepare(double sr)
        {
            this->sampleRate = sr;

            // Plate FDN delay times: shorter than room, near-prime spacing
            // These represent reflections within a compact metal plate
            const float baseDelays[NUM_CHANNELS] = {
                13.1f, 17.3f, 21.7f, 26.9f, 31.3f, 37.1f,
                41.9f, 47.3f, 53.1f, 59.7f, 67.3f, 73.1f
            };

            for (int i = 0; i < NUM_CHANNELS; ++i)
            {
                delayLines[i].prepare(sampleRate, 150.0);
                nominalDelayTimes[i] = baseDelays[i];

                lfos[i].prepare(sampleRate);
                // Wider LFO spread for lush plate modulation
                lfos[i].setFrequency(0.6f + (float)i * 0.07f);
                lfos[i].setDepth(0.0f);

                hiCutFilters[i].reset();
                loCutFilters[i].reset();
            }

            // Input diffusers for additional density
            for (auto& apf : inputDiffusersL) apf.prepare(sampleRate, 10.0);
            for (auto& apf : inputDiffusersR) apf.prepare(sampleRate, 10.0);

            reset();
        }

        void reset()
        {
            for (auto& d : delayLines) d.reset();
            std::fill(std::begin(outputs), std::end(outputs), 0.0f);
        }

        void setParameters(float decayTimeS, float modDepth, float modRate, float hiCut, float loCut)
        {
            // Feedback gain from T60 decay time
            // Plate average delay is shorter than room (~40ms)
            float avgDelayMs = 40.0f;
            feedbackGain = std::pow(0.001f, (avgDelayMs / 1000.0f) / decayTimeS);

            for (int i = 0; i < NUM_CHANNELS; ++i)
            {
                // Wider LFO variation for lush chorus-like modulation
                lfos[i].setFrequency(modRate * (0.8f + 0.04f * i));
                // Deeper modulation range (up to 5 samples) for plate shimmer
                lfos[i].setDepth(modDepth * 5.0f);

                hiCutFilters[i].setCoefficients(sampleRate, hiCut, OnePoleFilter::Type::LowPass);
                loCutFilters[i].setCoefficients(sampleRate, loCut, OnePoleFilter::Type::HighPass);
            }
        }

        void processBlock(juce::AudioBuffer<float>& buffer)
        {
            auto* left = buffer.getWritePointer(0);
            auto* right = buffer.getWritePointer(1);
            int numSamples = buffer.getNumSamples();

            for (int n = 0; n < numSamples; ++n)
            {
                float inL = left[n];
                float inR = right[n];

                // Read all delay outputs with modulation
                float delayOuts[NUM_CHANNELS];
                for (int i = 0; i < NUM_CHANNELS; ++i)
                {
                    float mod = lfos[i].process();
                    delayOuts[i] = delayLines[i].read(nominalDelayTimes[i] + mod);
                    outputs[i] = delayOuts[i];
                }

                // Householder mixing matrix: y = x - (2/N) * sum(x)
                float sum = 0.0f;
                for (int i = 0; i < NUM_CHANNELS; ++i)
                    sum += delayOuts[i];
                sum *= (2.0f / (float)NUM_CHANNELS);

                float feedbackOuts[NUM_CHANNELS];
                for (int i = 0; i < NUM_CHANNELS; ++i)
                {
                    float matrixOut = delayOuts[i] - sum;

                    // Distribute stereo input across channels
                    // L -> channels 0-5, R -> channels 6-11
                    float injection = (i < NUM_CHANNELS / 2) ? inL : inR;
                    feedbackOuts[i] = injection + matrixOut * feedbackGain;
                }

                // Filter and push back into delay lines
                for (int i = 0; i < NUM_CHANNELS; ++i)
                {
                    float processed = feedbackOuts[i];
                    processed = hiCutFilters[i].process(processed);
                    processed = loCutFilters[i].process(processed);
                    delayLines[i].push(processed);
                }

                // Stereo output: sum channel groups with alternating panning
                // More even distribution than strict L/R halves for wider image
                float outL = 0.0f;
                float outR = 0.0f;

                outL += delayOuts[0] + delayOuts[2] + delayOuts[4]
                      + delayOuts[7] + delayOuts[9] + delayOuts[11];
                outR += delayOuts[1] + delayOuts[3] + delayOuts[5]
                      + delayOuts[6] + delayOuts[8] + delayOuts[10];

                // Scale factor for 12-channel sum
                left[n]  = outL * 0.22f;
                right[n] = outR * 0.22f;
            }
        }

    private:
        double sampleRate = 44100.0;

        std::array<DelayLine, NUM_CHANNELS> delayLines;
        std::array<LFO, NUM_CHANNELS> lfos;
        float nominalDelayTimes[NUM_CHANNELS];
        float outputs[NUM_CHANNELS];

        std::array<AllpassFilter, 2> inputDiffusersL;
        std::array<AllpassFilter, 2> inputDiffusersR;

        std::array<OnePoleFilter, NUM_CHANNELS> hiCutFilters;
        std::array<OnePoleFilter, NUM_CHANNELS> loCutFilters;

        float feedbackGain = 0.5f;
    };
}
