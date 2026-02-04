#include <JuceHeader.h>
#include "../Source/DSP/AllpassFilter.h"
#include "../Source/DSP/LFO.h"

class DSPTests : public juce::UnitTest
{
public:
    DSPTests() : juce::UnitTest("DSP Components Tests") {}

    void runTest() override
    {
        beginTest("Allpass Filter");
        {
            DSP::AllpassFilter apf;
            apf.prepare(44100.0, 10.0); // 10ms
            
            // Impulse response check
            // Input: 1, 0, 0...
            // Output: -g, 1-g^2, ...
            float g = 0.5f;
            apf.setFeedback(g);
            
            float out1 = apf.process(1.0f);
            // y[0] = -0.5 * w[0] + delayed[0]. delayed[0]=0.
            // w[0] = 1 + 0.5*0 = 1.
            // y[0] = -0.5 * 1 + 0 = -0.5.
            expectWithinAbsoluteError(out1, -0.5f, 0.0001f);
            
            // push 0s until delayMs
            int delaySamples = (int)(10.0 * 44.1);
            for (int i = 0; i < delaySamples; ++i)
            {
                // apf.process(0.0f);
            }
            
            // We can check energy conservation roughly or just stability
        }

        beginTest("LFO");
        {
            DSP::LFO lfo;
            lfo.prepare(100.0); // 100Hz SR
            lfo.setFrequency(1.0f); // 1Hz
            lfo.setDepth(1.0f);
            
            // process 0 -> sin(0) = 0
            expectWithinAbsoluteError(lfo.process(), 0.0f, 0.0001f);
            
            // process 1 -> sin(2pi * 1/100)
            float expected = (float)std::sin(2.0 * juce::MathConstants<double>::pi * 0.01);
            expectWithinAbsoluteError(lfo.process(), expected, 0.0001f);
        }
    }
};

static DSPTests dspTests;
