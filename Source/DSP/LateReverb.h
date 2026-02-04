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
     * @brief Late Reverb engine using 8-channel FDN.
     */
    class LateReverb
    {
    public:
        LateReverb() = default;

        void prepare(double sr)
        {
            this->sampleRate = sr;
            
            // FDN Delays: Prime numbers around 30-100ms
            const float baseDelays[8] = { 29.1f, 37.3f, 44.9f, 53.7f, 61.3f, 79.1f, 88.7f, 97.1f };
            
            for (int i = 0; i < 8; ++i)
            {
                delayLines[i].prepare(sampleRate, 200.0); // Alloc enough buffer
                nominalDelayTimes[i] = baseDelays[i];
                
                lfos[i].prepare(sampleRate);
                lfos[i].setFrequency(0.5f + (float)i * 0.05f); // Spread LFO rates slightly
                lfos[i].setDepth(0.0f);
                
                hiCutFilters[i].reset();
                loCutFilters[i].reset();
            }
            
            // Input diffusers
            for (auto& apf : inputDiffusersL) apf.prepare(sampleRate, 13.0);
            for (auto& apf : inputDiffusersR) apf.prepare(sampleRate, 13.0);
            
            reset();
        }
        
        void reset()
        {
            for (auto& d : delayLines) d.reset();
            std::fill(std::begin(outputs), std::end(outputs), 0.0f);
        }

        void setParameters(float decayTimeS, float modDepth, float modRate, float hiCut, float loCut)
        {
            // Calculate feedback gain from decay time (T60)
            float avgDelayMs = 60.0f; // Approx
            feedbackGain = std::pow(0.001f, (avgDelayMs / 1000.0f) / decayTimeS);
            
            // Modulation
            for (int i = 0; i < 8; ++i)
            {
                lfos[i].setFrequency(modRate * (0.9f + 0.02f * i)); // Slight variation
                lfos[i].setDepth(modDepth * 3.0f); // Max 3 samples shift
                
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
                
                // Diffuse Input
                // ... (simplified loop for brevity)
                
                // FDN mixing (Hadamard-like or Householder)
                // New inputs to delays = Input + Matrix * DelayedValues
                // Simple Householder: y = x - 2/N * sum(x)
                
                // Read from all delays first
                float delayOuts[8];
                for (int i = 0; i < 8; ++i)
                {
                   float mod = lfos[i].process();
                   delayOuts[i] = delayLines[i].read(nominalDelayTimes[i] + mod);
                   outputs[i] = delayOuts[i]; // Store filter state if needed
                }
                
                // Matrix (Householder)
                // input vector 'x' is delayOuts scaled by feedback + new input
                
                float sum = 0.0f;
                for (int i = 0; i < 8; ++i) sum += delayOuts[i];
                sum *= (2.0f / 8.0f);
                
                float feedbackOuts[8];
                for (int i = 0; i < 8; ++i)
                {
                    float matrixOut = delayOuts[i] - sum; // Householder
                    // Add input (distribute to channels)
                    // L -> 0,1,2,3. R -> 4,5,6,7
                    float injection = (i < 4) ? inL : inR;
                    feedbackOuts[i] = injection + matrixOut * feedbackGain;
                }
                
                // Filter and Push
                for (int i = 0; i < 8; ++i)
                {
                    float processed = feedbackOuts[i];
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
        float outputs[8];
        
        std::array<AllpassFilter, 2> inputDiffusersL;
        std::array<AllpassFilter, 2> inputDiffusersR;

        std::array<OnePoleFilter, 8> hiCutFilters;
        std::array<OnePoleFilter, 8> loCutFilters;

        float feedbackGain = 0.5f;
    };
}
