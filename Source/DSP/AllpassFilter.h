#pragma once

#include "DelayLine.h"

namespace DSP
{
    /**
     * @brief Schroeder Allpass Filter: y[n] = -g * x[n] + x[n-D] + g * y[n-D]
     * Equivalent to: 
     * w = x + g * delay_out
     * y = delay_out - g * w
     * delay_in = w
     */
    class AllpassFilter
    {
    public:
        AllpassFilter() = default;

        void prepare(double sr, double ms)
        {
            delay.prepare(sr, ms);
            this->delayMs = (float)ms;
        }

        void setFeedback(float g)
        {
            feedback = g;
        }

        float process(float input)
        {
            float delayed = delay.read(delayMs);
            
            // Canonical form
            // w[n] = x[n] + g * y[n-D] (which is delayed)
            float w = input + feedback * delayed;
            
            // y[n] = -g * w[n] + y[n-D]
            float output = -feedback * w + delayed;
            
            // Update delay buffer
            delay.push(w);

            return output;
        }

    private:
        DelayLine delay;
        float delayMs = 0.0f;
        float feedback = 0.5f;
    };
}
