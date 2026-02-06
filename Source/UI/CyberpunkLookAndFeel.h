#pragma once
#include <JuceHeader.h>

namespace UI {
class CyberpunkLookAndFeel : public juce::LookAndFeel_V4 {
public:
  CyberpunkLookAndFeel() {
    // Cyberpunk Palette
    setColour(juce::ResizableWindow::backgroundColourId,
              juce::Colour(0xff1a1a1a)); // Deep Charcoal
    setColour(juce::Slider::trackColourId, juce::Colours::black);
    setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    setColour(juce::Label::textColourId, juce::Colours::white);
  }

  //==============================================================================
  // NEON FADERS (Linear)
  //==============================================================================
  void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider &slider) override {
    if (style == juce::Slider::LinearVertical) {
      auto trackWidth = width * 0.3f;
      auto trackX = x + (width - trackWidth) * 0.5f;

      // 1. Background Track (Dark slot)
      g.setColour(juce::Colours::black);
      g.fillRect((float)trackX, (float)y, trackWidth, (float)height);
      g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
      g.drawRect((float)trackX, (float)y, trackWidth, (float)height, 1.0f);

      // 2. Neon Bar (Value)
      // Map value to height
      auto valPos = slider.valueToProportionOfLength(slider.getValue());
      auto fillHeight = valPos * height;

      // Gradient: Cyan (bottom) to Magenta (top)
      juce::Colour cyan = juce::Colour(0xff00FFFF);
      juce::Colour magenta = juce::Colour(0xffFF00FF);
      juce::ColourGradient grad(cyan, trackX, y + height, magenta, trackX, y,
                                false);

      g.setGradientFill(grad);
      // Draw from bottom up
      g.fillRect((float)trackX, (float)(y + height - fillHeight), trackWidth,
                 (float)fillHeight);

      // 3. Digital Readout (Bottom overlay or inside?)
      // User requested "digital value readouts at the bottom".
      // We'll draw it below the slider in the parent editor usually, but we can
      // draw it on the handle if we wanted. The prompt implies the slider
      // *component* has the readout. We'll leave the text drawing to the Label
      // or TextBox attached.
    } else {
      // Fallback for others
      juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                             minSliderPos, maxSliderPos, style,
                                             slider);
    }
  }

  //==============================================================================
  // NEON KNOBS (Rotary)
  //==============================================================================
  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider &slider) override {
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    // 1. Background (Dark Circle)
    g.setColour(juce::Colour(0xff111111));
    g.fillEllipse(rx, ry, rw, rw);
    g.setColour(juce::Colours::black); // rim
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    // 2. Neon Arc
    // Value
    auto angle = rotaryStartAngle +
                 sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path p;
    juce::Path stroke;

    // Outer Ring Glow
    g.setColour(juce::Colour(0xff00FFFF)); // Electric Cyan

    // Draw arc
    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius * 0.85f, radius * 0.85f, 0.0f,
                      rotaryStartAngle, angle, true);

    g.strokePath(arc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

    // 3. Center Digital Readout
    // Show current value as Hex/Numeric?
    // "central digital readout (hex/numeric)"
    // Let's verify if the slider has a textbox covering this.
    // If the slider has TextBox::NoTextBox, we draw it here.
    // If it has a visible textbox, we might clash.
    // Ideally we draw the text here for that "embedded" look.

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);

    juce::String displayVal;
    // Hacky hex/numeric mix:
    // If value < 10, show normally. If > 10, maybe show Hex?
    // Providing simple numeric for now, heavily styled.

    displayVal = juce::String(slider.getValue(), 1); // 1 decimal place

    g.drawText(displayVal, x, y, width, height, juce::Justification::centred,
               false);

    // Draw Label (Name) if needed? No, separate labels usually.
  }

  //==============================================================================
  // TOGGLE BUTTON (Mode)
  //==============================================================================
  void drawButtonBackground(juce::Graphics &g, juce::Button &button,
                            const juce::Colour &backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override {
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = 4.0f;

    // Neon Accents
    juce::Colour base = juce::Colour(0xff2a2a2a);
    juce::Colour glow = juce::Colour(0xffFF00FF); // Magenta

    if (button.getToggleState() || shouldDrawButtonAsDown) {
      base = glow.withAlpha(0.2f);
      g.setColour(glow);
      g.drawRect(bounds, 2.0f); // Bright border
    } else {
      g.setColour(juce::Colours::darkgrey);
      g.drawRect(bounds, 1.0f);
    }

    g.setColour(base);
    g.fillRect(bounds.reduced(2));
  }

  void drawButtonText(juce::Graphics &g, juce::TextButton &button,
                      bool /*shouldDrawButtonAsHighlighted*/,
                      bool /*shouldDrawButtonAsDown*/) override {
    g.setFont(14.0f);
    g.setColour(button
                    .findColour(button.getToggleState()
                                    ? juce::TextButton::textColourOnId
                                    : juce::TextButton::textColourOffId)
                    .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    g.drawText(button.getButtonText(), button.getLocalBounds().reduced(2),
               juce::Justification::centred, true);
  }
};
} // namespace UI
