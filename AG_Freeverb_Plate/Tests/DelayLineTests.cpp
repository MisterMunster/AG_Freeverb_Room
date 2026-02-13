#include <JuceHeader.h>
#include "../Source/DSP/DelayLine.h"

class DelayLineTests : public juce::UnitTest
{
public:
    DelayLineTests() : juce::UnitTest("DelayLine Tests") {}

    void runTest() override
    {
        beginTest("Basic Delay Functionality");

        DSP::DelayLine delayLine;
        double sampleRate = 1000.0; // 1ms = 1 sample
        delayLine.prepare(sampleRate, 100.0); // 100ms buffer

        delayLine.reset();

        // t=0: push(1.0). write=1. buffer[0]=1.
        delayLine.push(1.0f);

        // read(1ms) = 1 sample delay -> should read buffer[0] = 1.0
        expectEquals(delayLine.read(1.0f), 1.0f);

        // Check fractional delay with linear interpolation
        // push(2.0). write=2. buffer[1]=2.0.
        // read(1.5ms) -> 1.5 samples -> interpolate buffer[0]=1.0 and buffer[1]=2.0
        delayLine.push(2.0f);
        expectEquals(delayLine.read(1.5f * 1000.0f / (float)sampleRate), 1.5f);
    }
};

static DelayLineTests delayLineTests;
