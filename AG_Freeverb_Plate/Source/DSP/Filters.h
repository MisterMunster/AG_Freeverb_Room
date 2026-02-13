#pragma once

#include <cmath>
#include <JuceHeader.h>

namespace DSP
{
    class OnePoleFilter
    {
    public:
        enum class Type { LowPass, HighPass };

        OnePoleFilter() = default;

        void reset() { z1 = 0.0f; x_prev = 0.0f; }

        void setCoefficients(double sampleRate, float frequency, Type filterType)
        {
            double w = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;

            if (filterType == Type::LowPass)
            {
                a1 = (float)std::exp(-w);
                b0 = 1.0f - a1;
                this->type = Type::LowPass;
            }
            else
            {
                a1 = (float)std::exp(-w);
                b0 = a1;
                this->type = Type::HighPass;
            }
        }

        float process(float input)
        {
            if (type == Type::LowPass)
            {
                z1 = input * b0 + z1 * a1;
                return z1;
            }
            else
            {
                float output = a1 * (z1 + input - x_prev);
                x_prev = input;
                z1 = output;
                return output;
            }
        }

    private:
        float a1 = 0.0f;
        float b0 = 1.0f;
        float z1 = 0.0f;
        float x_prev = 0.0f;
        Type type = Type::LowPass;
    };
}
