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

            float g = 0.5f;
            apf.setFeedback(g);

            float out1 = apf.process(1.0f);
            // y[0] = -g * w[0] + delayed[0]
            // w[0] = 1 + g*0 = 1
            // y[0] = -0.5 * 1 + 0 = -0.5
            expectWithinAbsoluteError(out1, -0.5f, 0.0001f);
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
