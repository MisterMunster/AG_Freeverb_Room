#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace DSP
{
    class LFO
    {
    public:
        enum class Waveform { Sine, Triangle };

        LFO() = default;

        void prepare(double sr)
        {
            this->sampleRate = sr;
            phase = 0.0;
        }

        void setFrequency(float freq)
        {
            frequency = freq;
            phaseIncrement = (frequency * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        }

        void setDepth(float d)
        {
            depth = d;
        }

        float process()
        {
            float out = (float)std::sin(phase) * depth;
            
            phase += phaseIncrement;
            if (phase >= 2.0 * juce::MathConstants<double>::pi)
                phase -= 2.0 * juce::MathConstants<double>::pi;
                
            return out;
        }

    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        double phaseIncrement = 0.0;
        float frequency = 0.5f;
        float depth = 1.0f;
    };
}
