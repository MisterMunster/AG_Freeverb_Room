#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"
#include "AllpassFilter.h"
#include <array>

namespace DSP
{
    /**
     * @brief Plate-style Early Reflections engine.
     *
     * Plate reverbs produce extremely dense, rapid early reflections because
     * sound travels fast through metal and bounces off edges at very short
     * intervals. This uses 5 cascaded allpass diffusers per channel (vs 3 for
     * room) with shorter delay times, and 6 taps per channel at tighter
     * intervals to create the characteristic instant-density plate onset.
     */
    class EarlyReflections
    {
    public:
        EarlyReflections() = default;

        void prepare(double sr)
        {
            this->sampleRate = sr;

            // 5 series allpass diffusers per channel for maximum density
            for (auto& apf : diffusersL) apf.prepare(sr, 12.0);
            for (auto& apf : diffusersR) apf.prepare(sr, 12.0);

            // Main delay lines
            delayL.prepare(sr, 300.0);
            delayR.prepare(sr, 300.0);

            reset();
        }

        void reset()
        {
            for (auto& apf : diffusersL) apf.setFeedback(0.0f);
            for (auto& apf : diffusersR) apf.setFeedback(0.0f);
            delayL.reset();
            delayR.reset();
        }

        void setParameters(float sizeMs, float cross, float diffusionAmt)
        {
            currentSizeMs = sizeMs;
            currentCross = cross;

            // Higher diffusion coefficient for plate density (max 0.7)
            float diff = diffusionAmt * 0.7f;
            for (auto& apf : diffusersL) apf.setFeedback(diff);
            for (auto& apf : diffusersR) apf.setFeedback(diff);

            // Plate diffuser delays: very short, tightly spaced
            // These create the characteristic "instant smear" of a plate
            static const float baseDelaysL[] = { 1.5f, 2.7f, 4.1f, 5.8f, 8.3f };
            static const float baseDelaysR[] = { 1.7f, 3.1f, 4.5f, 6.2f, 8.9f };

            for (size_t i = 0; i < 5; ++i)
            {
                diffusersL[i].prepare(sampleRate, baseDelaysL[i]);
                diffusersR[i].prepare(sampleRate, baseDelaysR[i]);
            }
        }

        void processBlock(juce::AudioBuffer<float>& buffer)
        {
            auto* left = buffer.getWritePointer(0);
            auto* right = buffer.getWritePointer(1);
            int numSamples = buffer.getNumSamples();

            for (int i = 0; i < numSamples; ++i)
            {
                float inL = left[i];
                float inR = right[i];

                // 1. Crossfeed Input - plates have strong inter-channel coupling
                float mixedL = inL * (1.0f - currentCross * 0.5f) + inR * (currentCross * 0.5f);
                float mixedR = inR * (1.0f - currentCross * 0.5f) + inL * (currentCross * 0.5f);

                // 2. Dense diffusion through 5 cascaded allpasses
                float diffL = mixedL;
                float diffR = mixedR;
                for (auto& apf : diffusersL) diffL = apf.process(diffL);
                for (auto& apf : diffusersR) diffR = apf.process(diffR);

                // 3. Push into delay lines
                delayL.push(diffL);
                delayR.push(diffR);

                // 4. Dense multi-tap output (6 taps per channel, tightly spaced)
                // Plate reflections arrive very quickly and densely
                float outL = 0.0f;
                float outR = 0.0f;

                outL += delayL.read(currentSizeMs * 0.05f) * 0.45f;
                outL += delayL.read(currentSizeMs * 0.15f) * 0.40f;
                outL += delayR.read(currentSizeMs * 0.28f) * 0.30f;  // Cross-tap
                outL += delayL.read(currentSizeMs * 0.41f) * 0.25f;
                outL += delayR.read(currentSizeMs * 0.58f) * 0.20f;  // Cross-tap
                outL += delayL.read(currentSizeMs * 0.76f) * 0.15f;

                outR += delayR.read(currentSizeMs * 0.07f) * 0.45f;
                outR += delayR.read(currentSizeMs * 0.18f) * 0.40f;
                outR += delayL.read(currentSizeMs * 0.32f) * 0.30f;  // Cross-tap
                outR += delayR.read(currentSizeMs * 0.45f) * 0.25f;
                outR += delayL.read(currentSizeMs * 0.62f) * 0.20f;  // Cross-tap
                outR += delayR.read(currentSizeMs * 0.81f) * 0.15f;

                left[i] = outL;
                right[i] = outR;
            }
        }

    private:
        double sampleRate = 44100.0;

        // 5 series allpasses per channel (vs 3 in room) for plate density
        std::array<AllpassFilter, 5> diffusersL;
        std::array<AllpassFilter, 5> diffusersR;

        DelayLine delayL;
        DelayLine delayR;

        float currentSizeMs = 100.0f;
        float currentCross = 0.3f;
    };
}
