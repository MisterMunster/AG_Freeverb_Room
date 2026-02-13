#include <JuceHeader.h>
#include "../Source/DSP/EarlyReflections.h"
#include "../Source/DSP/LateReverb.h"

class EngineTests : public juce::UnitTest
{
public:
    EngineTests() : juce::UnitTest("Plate Reverb Engine Tests") {}

    void runTest() override
    {
        beginTest("Plate Early Reflections Processing");
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
                if (std::isnan(buffer.getSample(1, i)) || std::isinf(buffer.getSample(1, i))) valid = false;
            }
            expect(valid, "Plate Early Reflections produced valid output (no NaN/Inf)");
        }

        beginTest("Plate Late Reverb Processing");
        {
            DSP::LateReverb lr;
            lr.prepare(44100.0);
            lr.setParameters(3.0f, 0.5f, 0.8f, 10000.0f, 40.0f);

            juce::AudioBuffer<float> buffer(2, 512);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f); // Impulse

            lr.processBlock(buffer);

            bool valid = true;
            for (int i = 0; i < 512; ++i)
            {
                if (std::isnan(buffer.getSample(0, i))) valid = false;
                if (std::isnan(buffer.getSample(1, i))) valid = false;
            }
            expect(valid, "Plate Late Reverb produced valid output");

            // Process more blocks to let FDN feedback build up
            lr.processBlock(buffer);
            lr.processBlock(buffer);

            float maxVal = buffer.getMagnitude(0, 512);
            expect(maxVal > 0.0f, "Plate reverb tail should be present");
        }

        beginTest("Plate 12-Channel FDN Density");
        {
            // Verify the 12-channel FDN produces output on both channels
            DSP::LateReverb lr;
            lr.prepare(44100.0);
            lr.setParameters(2.0f, 0.3f, 0.5f, 12000.0f, 30.0f);

            juce::AudioBuffer<float> buffer(2, 1024);
            buffer.clear();
            buffer.setSample(0, 0, 1.0f);
            buffer.setSample(1, 0, 1.0f);

            // Process several blocks to allow feedback
            for (int block = 0; block < 4; ++block)
            {
                lr.processBlock(buffer);
            }

            float magL = buffer.getMagnitude(0, 0, 1024);
            float magR = buffer.getMagnitude(1, 0, 1024);

            expect(magL > 0.0f, "Left channel should have energy from 12-ch FDN");
            expect(magR > 0.0f, "Right channel should have energy from 12-ch FDN");
        }
    }
};

static EngineTests engineTests;
