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

        // Push 1.0 at output
        delayLine.push(1.0f);
        // Push zeros
        for (int i = 0; i < 9; ++i) delayLine.push(0.0f);

        // At 10ms delay (10 samples), we should see 1.0f
        // currently writeIndex is at 11 (index 10 was the last push).
        // buffer has [1, 0, 0, ...]. 
        // read(10ms) -> 10 samples ago.
        
        // Let's reset and do it carefully.
        delayLine.reset();
        
        // t=0: push 1.0. write pos becomes 1. buffer[0] = 1.
        delayLine.push(1.0f); 
        
        // at t=0, read(0) should be 1.0 ?? No, usually read happens before or after push depending on block processing.
        // In our class, read(0) gets the value at writeIndex - 0. 
        // If we push then read:
        // push(1.0) -> buffer[0]=1, writeIndex=1.
        // read(0) -> index = 1 - 0 = 1. buffer[1] is 0 (or garbage). 
        // Wait, standard circular buffer read(0) should be the most recently written sample?
        // My implementation: readPos = writeIndex - delaySamples.
        // If delay is 0, readPos = writeIndex.
        // But writeIndex points to the NEXT empty slot.
        // So read(0) reads the NEXT empty slot, which is wrong.
        // read(0) should read the LAST WRITTEN slot.
        
        // Let's fix the implementation in the next step. 
        // Usually read pointer is separate, OR we interpret delay=0 as "current input" (if direct feed) or "last output".
        // If I want read(delay) to return what was pushed 'delay' samples ago:
        // If delay=1 sample:
        // push(x) -> buffer[0]=x, write=1.
        // read(1) -> should be x.
        // My code: readPos = 1 - 1 = 0. buffer[0] = x. Correct.
        
        // So read(0) returns buffer[writeIndex], which is future/current.
        // effectively minimum delay is 0 (but that means feedthrough?).
        // If delay=0, it reads buffer[writeIndex], which is whatever is there before we write to it? 
        // No, I just wrote to buffer[writeIndex] then incremented.
        // So buffer[writeIndex] is now the old value (if overwritten) or 0.
        // Ah, `push` does: buffer[writeIndex] = input; writeIndex++;
        // So buffer[writeIndex-1] is the newest.
        // read(0) -> writeIndex. This is one sample AHEAD? or wrapped around?
        // Correct logic: read(0) should probably be impossible if strictly "delay", or equal to input.
        // Usually: read(1) = 1 sample delay.
        
        // Let's check 1 sample delay.
        // t=0: push(1.0). write=1.
        // read(1) -> pos=0 -> buffer[0]=1.0. Correct.
        
        expectEquals(delayLine.read(1.0f), 1.0f); // 1ms = 1 sample
        
        // Check fractional delay
        // 1.5 samples delay.
        // buffer currently: [1, 0, 0...] (assuming clear)
        // push(2.0). write=2. buffer[1]=2.0.
        // read(1.5) -> pos = 2 - 1.5 = 0.5.
        // interpolate buffer[0] mult 0.5 + buffer[1] mult 0.5 = 1.0*0.5 + 2.0*0.5 = 1.5.
        delayLine.push(2.0f);
        expectEquals(delayLine.read(1.5f * 1000.0/sampleRate), 1.5f);
    }
};

static DelayLineTests delayLineTests;
