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

        void reset() { z1 = 0.0f; }

        void setCoefficients(double sampleRate, float frequency, Type filterType)
        {
            // Simple 1-pole mapping
            // w = 2*pi*f/sr
            // b1 = exp(-w) for LP?
            // Using JUCE's IIR logic or basic equations.
            
            // Standard one-pole LPF: y[n] = b0*x[n] + a1*y[n-1]
            // a1 = exp(-2*pi*f/sr)
            // b0 = 1 - a1
            
            double w = 2.0 * juce::MathConstants<double>::pi * frequency / sampleRate;
            
            if (filterType == Type::LowPass)
            {
                a1 = (float)std::exp(-w);
                b0 = 1.0f - a1;
                this->type = Type::LowPass;
            }
            else
            {
                // HPF
                // y[n] = a1 * (y[n-1] + x[n] - x[n-1])
                // a1 = exp(-w)
                a1 = (float)std::exp(-w);
                b0 = a1; // Used differently in process
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
                // HPF: y[n] = a1 * (y[n-1] + x[n] - x[n-1])
                // z1 holds y[n-1]? or x[n-1]?
                // Let's use direct form 1 or similar.
                // y[n] = (x[n] - x[n-1]) * (1+a1)/2 + y[n-1]*a1 ?
                // Simpler: y[n] = input - lpf(input)
                // But efficient direct calculation:
                // y[n] = a1 * (y[n-1] + input - x_prev)
                float output = a1 * (z1 + input - x_prev);
                x_prev = input;
                z1 = output;
                return output;
            }
        }

    private:
        float a1 = 0.0f;
        float b0 = 1.0f;
        float z1 = 0.0f; // y[n-1] for LPF, y[n-1] for HPF
        float x_prev = 0.0f; // x[n-1] for HPF
        Type type = Type::LowPass;
    };
}
