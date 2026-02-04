#include <JuceHeader.h>
#include "../Source/DSP/EarlyReflections.h"
#include "../Source/DSP/LateReverb.h"

class EngineTests : public juce::UnitTest
{
public:
    EngineTests() : juce::UnitTest("Reverb Engine Tests") {}

    void runTest() override
    {
        beginTest("Early Reflections Processing");
        {
            DSP::EarlyReflections er;
            er.prepare(44100.0);
            
            juce::AudioBuffer<float> buffer(2, 512);
            buffer.clear();
            
            // Inject impulse
            buffer.setSample(0, 0, 1.0f);
            buffer.setSample(1, 0, 0.5f);
            
            er.processBlock(buffer);
            
            // Check for NaNs or Infinity
            bool valid = true;
            for (int i = 0; i < 512; ++i)
            {
                if (std::isnan(buffer.getSample(0, i)) || std::isinf(buffer.getSample(0, i))) valid = false;
            }
            expect(valid, "Early Reflections produced valid output (no NaN/Inf)");
        }
        
        beginTest("Late Reverb Processing");
        {
            DSP::LateReverb lr;
            lr.prepare(44100.0);
            lr.setParameters(2.0f, 0.5f, 0.5f, 10000.0f, 50.0f);
            
            juce::AudioBuffer<float> buffer(2, 512);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f); // Impulse
            
            lr.processBlock(buffer);
            
            bool valid = true;
            for (int i = 0; i < 512; ++i)
            {
                 if (std::isnan(buffer.getSample(0, i))) valid = false;
            }
            expect(valid, "Late Reverb produced valid output");
            
            // Expect some output energy later in buffer due to delays (FDN min delay ~30ms)
            // 30ms at 44.1k is ~1300 samples. 
            // Wait, my buffer is 512.
            // process again.
            lr.processBlock(buffer); // 1024
            lr.processBlock(buffer); // 1536
            
            // Should see something non-zero now if feedback works.
            float maxVal = buffer.getMagnitude(0, 512);
            expect(maxVal > 0.0f, "Reverb tail should be present");
        }
    }
};

static EngineTests engineTests;
