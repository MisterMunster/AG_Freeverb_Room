#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

namespace DSP
{

    /**
     * @brief A fractional delay line with linear interpolation.
     */
    class DelayLine
    {
    public:
        DelayLine() = default;

        void prepare(double newSampleRate, double maxDelayMs)
        {
            this->sampleRate = newSampleRate;
            buffer.resize((size_t)std::ceil(maxDelayMs * sampleRate / 1000.0) + 1);
            reset();
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
        }

        void push(float input)
        {
            buffer[writeIndex] = input;
            writeIndex = (writeIndex + 1) % buffer.size();
        }

        /**
         * @brief Read from the delay line at a specific delay time.
         * @param delayMs Delay time in milliseconds.
         */
        float read(float delayMs) const
        {
            float delaySamples = delayMs * (float)sampleRate / 1000.0f;
            
            // Clamp delay to valid range [0, bufferSize - 1]
            // In a real circular buffer, we effectively want delaySamples < buffer.size()
            if (delaySamples < 0.0f) delaySamples = 0.0f;
            if (delaySamples >= buffer.size() - 1) delaySamples = (float)buffer.size() - 1.01f;

            float readPos = (float)writeIndex - delaySamples;
            
            // Wrap readPos
            while (readPos < 0.0f) readPos += (float)buffer.size();
            while (readPos >= (float)buffer.size()) readPos -= (float)buffer.size();

            // Linear interpolation
            int index1 = (int)readPos;
            int index2 = (index1 + 1) % buffer.size();
            float frac = readPos - (float)index1;

            return buffer[index1] + frac * (buffer[index2] - buffer[index1]);
        }

    private:
        std::vector<float> buffer;
        size_t writeIndex = 0;
        double sampleRate = 44100.0;
    };

} // namespace DSP
