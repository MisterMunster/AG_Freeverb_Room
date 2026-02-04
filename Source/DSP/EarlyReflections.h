#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"
#include "AllpassFilter.h"
#include <array>

namespace DSP
{
    /**
     * @brief Early Reflections engine.
     * Uses input diffusion (series allpasses) followed by a multi-tap delay structure.
     */
    class EarlyReflections
    {
    public:
        EarlyReflections() = default;

        void prepare(double sr)
        {
            this->sampleRate = sr;
            
            // Initialize diffusers
            for (auto& apf : diffusersL) apf.prepare(sr, 10.0); // Arbitrary max
            for (auto& apf : diffusersR) apf.prepare(sr, 10.0);
            
            // Initialize main delays
            delayL.prepare(sr, 500.0); // Max 500ms size
            delayR.prepare(sr, 500.0);
            
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
            
            // Update diffusion coefficients
            float diff = diffusionAmt * 0.6f; // Max correlation
            for (auto& apf : diffusersL) apf.setFeedback(diff);
            for (auto& apf : diffusersR) apf.setFeedback(diff);
            
            // Update diffuser delay times (scaled by size generally, or fixed for character)
            // Sticking to fixed decent prime numbers for "smearing" or scaled.
            // Let's scale them slightly with size to keep density consistent relative to room.
            // Values: 4.7, 8.3, 11.1 ms etc.
            static const float baseDelays[] = { 4.3f, 7.1f, 13.7f };
            for (size_t i = 0; i < 3; ++i)
            {
                // Keeping diffusers relatively short/fixed usually better for ER
                diffusersL[i].prepare(sampleRate, baseDelays[i]);
                diffusersR[i].prepare(sampleRate, baseDelays[i] + 2.3f); // Decorrelate L/R
            }
        }

        // Processing stereo block
        void processBlock(juce::AudioBuffer<float>& buffer)
        {
            auto* left = buffer.getWritePointer(0);
            auto* right = buffer.getWritePointer(1);
            int numSamples = buffer.getNumSamples();

            for (int i = 0; i < numSamples; ++i)
            {
                float inL = left[i];
                float inR = right[i];
                
                // 1. Crossfeed Input
                float mixedL = inL * (1.0f - currentCross * 0.5f) + inR * (currentCross * 0.5f);
                float mixedR = inR * (1.0f - currentCross * 0.5f) + inL * (currentCross * 0.5f);
                
                // 2. Diffusion
                float diffL = mixedL;
                float diffR = mixedR;
                for (auto& apf : diffusersL) diffL = apf.process(diffL);
                for (auto& apf : diffusersR) diffR = apf.process(diffR);
                
                // 3. Delay Line Input
                delayL.push(diffL);
                delayR.push(diffR);
                
                // 4. Taps output
                // Taps at ratios of currentSizeMs.
                // E.g. 0.1, 0.3, 0.5, 0.8...
                
                float outL = 0.0f;
                float outR = 0.0f;
                
                // Simple pattern
                // L: taps at 0.1, 0.4, 0.7
                // R: taps at 0.2, 0.5, 0.9
                
                outL += delayL.read(currentSizeMs * 0.11f) * 0.6f;
                outL += delayL.read(currentSizeMs * 0.43f) * 0.4f;
                outL += delayR.read(currentSizeMs * 0.67f) * 0.3f; // Cross-tap
                outL += delayL.read(currentSizeMs * 0.91f) * 0.2f;

                outR += delayR.read(currentSizeMs * 0.13f) * 0.6f;
                outR += delayR.read(currentSizeMs * 0.47f) * 0.4f;
                outR += delayL.read(currentSizeMs * 0.71f) * 0.3f; // Cross-tap
                outR += delayR.read(currentSizeMs * 0.97f) * 0.2f;
                
                left[i] = outL;
                right[i] = outR;
            }
        }

    private:
        double sampleRate = 44100.0;
        
        std::array<AllpassFilter, 3> diffusersL;
        std::array<AllpassFilter, 3> diffusersR; // 3 series allpasses per channel
        
        DelayLine delayL;
        DelayLine delayR;
        
        float currentSizeMs = 300.0f;
        float currentCross = 0.1f;
    };
}
