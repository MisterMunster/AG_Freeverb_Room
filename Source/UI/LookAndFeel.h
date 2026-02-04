#pragma once

#include <JuceHeader.h>

namespace UI
{
    class DarkLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        DarkLookAndFeel()
        {
            // Colors
            setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(20, 20, 24));
            setColour(juce::Slider::thumbColourId, juce::Colour(255, 60, 60)); // Red accent?
            // "Early" default highlighted in red.
            // Let's use a sleek dark grey/black with orange/red accents.
            
            setColour(juce::Slider::trackColourId, juce::Colour(40, 40, 45));
            setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(200, 50, 50));
            setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(30, 30, 35));
            setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                               const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& /*slider*/) override
        {
            auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
            auto centreX = (float) x + (float) width  * 0.5f;
            auto centreY = (float) y + (float) height * 0.5f;
            auto rx = centreX - radius;
            auto ry = centreY - radius;
            auto rw = radius * 2.0f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            // Background
            g.setColour (findColour (juce::Slider::rotarySliderOutlineColourId));
            g.fillEllipse (rx, ry, rw, rw);
            
            // Arc
            juce::Path p;
            p.addArc (rx, ry, rw, rw, rotaryStartAngle, angle, true);
            g.setColour (findColour (juce::Slider::rotarySliderFillColourId));
            g.strokePath (p, juce::PathStrokeType (radius * 0.2f)); // Thick arc
            
            // Pointer or internal
        }

        void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                               const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            // Vertical bar style
            g.fillAll (slider.findColour (juce::Slider::backgroundColourId));

            auto trackWidth = juce::jmin (6.0f, (float) width * 0.25f);
            // juce::Point<float> startPoint ... unused? No warning yet.
            
            if (style == juce::Slider::LinearVertical)
            {
                 // Draw track
                 g.setColour (findColour (juce::Slider::trackColourId));
                 g.fillRect ((float)x + width*0.5f - trackWidth*0.5f, (float)y, trackWidth, (float)height);
                 
                 // Draw Fill
                 g.setColour (findColour (juce::Slider::thumbColourId));
                 
                 juce::Rectangle<float> fillRect ((float)x + width*0.5f - trackWidth*0.5f, sliderPos, trackWidth, (float)y + (float)height - sliderPos);
                 g.fillRect(fillRect);
            }
        }
    };
}
