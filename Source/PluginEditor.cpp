#include "PluginEditor.h"

//==============================================================================
AntigravReverbAudioProcessorEditor::AntigravReverbAudioProcessorEditor(
    AntigravReverbAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Apply Cyberpunk Theme
  juce::LookAndFeel::setDefaultLookAndFeel(&cyberpunkLnF);

  // Helper for Faders (Left Panel)
  auto setupFader = [this](juce::Slider &s,
                           std::unique_ptr<SliderAttachment> &att,
                           const juce::String &pID) {
    s.setSliderStyle(juce::Slider::LinearVertical);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    s.setColour(juce::Slider::textBoxOutlineColourId,
                juce::Colours::transparentBlack);
    addAndMakeVisible(s);
    att = std::make_unique<SliderAttachment>(audioProcessor.apvts, pID, s);
  };

  // Helper for Neon Knobs (Right Panel)
  auto setupKnob = [this](juce::Slider &s,
                          std::unique_ptr<SliderAttachment> &att,
                          const juce::String &pID) {
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0,
                      0); // Readout is drawn by LNF
    addAndMakeVisible(s);
    att = std::make_unique<SliderAttachment>(audioProcessor.apvts, pID, s);
  };

  // Helper for Labels
  auto setupLabel = [this](juce::Label &l, const juce::String &text,
                           int fontSize) {
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont((float)fontSize);
    addAndMakeVisible(l);
  };

  // --- LEFT PANEL COMPONENTS (Faders) ---
  // Row 1
  setupFader(mixSlider, mixAtt, Params::mix);
  setupLabel(mixLabel, "MIX", 14);
  setupFader(predelaySlider, predelayAtt, Params::predelay);
  setupLabel(predelayLabel, "PRE", 14);
  setupFader(decaySlider, decayAtt, Params::decay);
  setupLabel(decayLabel, "DECAY", 14);
  // Row 2
  setupFader(loCutSlider, loCutAtt, Params::loCut);
  setupLabel(loCutLabel, "LO CUT", 14);
  setupFader(hiCutSlider, hiCutAtt, Params::hiCut);
  setupLabel(hiCutLabel, "HI CUT", 14);
  setupFader(depthSlider, depthAtt, Params::modDepth);
  setupLabel(depthLabel, "DEPTH", 14);

  // --- RIGHT PANEL COMPONENTS (Knobs) ---
  // Row 1
  setupKnob(earlySizeKnob, eSizeAtt, Params::earlySize);
  setupLabel(earlySizeLbl, "SIZE", 16);
  setupKnob(earlyCrossKnob, eCrossAtt, Params::earlyCross);
  setupLabel(earlyCrossLbl, "CROSS", 16);
  setupKnob(diffKnob, diffAtt, Params::diffusion);
  setupLabel(diffLbl, "DIFF", 16);
  // Row 2
  setupKnob(modRateKnob, mRateAtt, Params::modRate);
  setupLabel(modRateLbl, "RATE", 16);
  setupKnob(modDepthKnob, mDepthAtt, Params::modDepthSub);
  setupLabel(modDepthLbl, "MOD LV", 16); // Using SubDepth for knob?
  setupKnob(earlySendKnob, eSendAtt, Params::earlySend);
  setupLabel(earlySendLbl, "SEND", 16);

  // --- HEADER ---
  addAndMakeVisible(modeButton);
  modeButton.setClickingTogglesState(true);
  modeButton.addListener(this);
  // Button styling handled by LNF

  showLate = false;
  modeButton.setToggleState(false, juce::dontSendNotification);

  setSize(900, 600); // Slightly larger for "Large" knobs
}

AntigravReverbAudioProcessorEditor::~AntigravReverbAudioProcessorEditor() {
  juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void AntigravReverbAudioProcessorEditor::buttonClicked(juce::Button *button) {
  if (button == &modeButton) {
    showLate = modeButton.getToggleState();
    modeButton.setButtonText(showLate ? "MODE: LATE" : "MODE: EARLY");

    // In this Cyberpunk layout, the specific request was for a consistent grid
    // "Right Panel: Create a main control grid... Labels are: Size, Cross,
    // Diff, Rate, Mod Lv, Send." These labels imply a STATIC set of controls
    // (Early+Late params mixed?). If the controls are static, I don't need to
    // show/hide anything based on Mode. But I'll keep the Mode button as
    // requested ("Header: ... Mode toggle"). Maybe it changes the visual
    // context or routing, but if the controls are fixed, I'll just leave them
    // visible. For now, I will NOT hide the knobs, as the user asked for a
    // specific grid of 6.
  }
}

void AntigravReverbAudioProcessorEditor::paint(juce::Graphics &g) {
  // 1. Matte Black Textured Background
  g.fillAll(juce::Colour(0xff121212));

  // Simple noise texture effect
  /*
  g.setColour(juce::Colours::white.withAlpha(0.02f));
  for (int i=0; i < getHeight(); i+=2)
      g.drawHorizontalLine(i, 0, (float)getWidth());
  */

  // 2. Panel Dividers
  auto area = getLocalBounds();
  auto header = area.removeFromTop(60);
  auto leftPanel = area.removeFromLeft((int)(getWidth() * 0.35f));
  // Right panel is remainder

  // Draw Header
  g.setColour(juce::Colour(0xff00FFFF)); // Neon Cyan
  g.setFont(juce::Font("Impact", 28.0f, juce::Font::plain));
  g.drawText("FREE-VERB-ROOM", header.reduced(20, 0),
             juce::Justification::centredLeft, true);

  // Divider Lines
  g.setColour(juce::Colour(0xff2a2a2a)); // Dark grey
  g.drawVerticalLine(leftPanel.getRight(), (float)area.getY(),
                     (float)area.getBottom());
  g.drawHorizontalLine(header.getBottom(), 0.0f, (float)getWidth());

  // Glowing borders for containers?
  // g.setColour(juce::Colour(0xffFF00FF).withAlpha(0.1f));
  // g.drawRect(leftPanel.reduced(10), 2.0f);
}

void AntigravReverbAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  auto header = area.removeFromTop(60);

  // Mode Button Top Right
  modeButton.setBounds(header.removeFromRight(150).reduced(10));

  // Panels
  auto leftPanel = area.removeFromLeft((int)(getWidth() * 0.35f));
  auto rightPanel = area;

  // --- LEFT PANEL (Faders) ---
  // 2 Rows, 3 Cols
  int faderW = leftPanel.getWidth() / 3;
  int faderH = leftPanel.getHeight() / 2;

  auto placeFader = [&](juce::Slider &s, juce::Label &l, int c, int r) {
    auto bounds =
        juce::Rectangle<int>(leftPanel.getX() + c * faderW,
                             leftPanel.getY() + r * faderH, faderW, faderH)
            .reduced(5);
    // Label at top? User: "digital value readouts at the bottom".
    // My setupFader put text box at bottom. Label usually name. I'll put Label
    // at TOP.
    l.setBounds(bounds.removeFromTop(20));
    s.setBounds(bounds);
  };

  // Row 1
  placeFader(mixSlider, mixLabel, 0, 0);
  placeFader(predelaySlider, predelayLabel, 1, 0);
  placeFader(decaySlider, decayLabel, 2, 0);

  // Row 2
  placeFader(loCutSlider, loCutLabel, 0, 1);
  placeFader(hiCutSlider, hiCutLabel, 1, 1);
  placeFader(depthSlider, depthLabel, 2, 1);

  // --- RIGHT PANEL (Knobs) ---
  // 2 Rows, 3 Cols
  int knobW = rightPanel.getWidth() / 3;
  int knobH = rightPanel.getHeight() / 2;

  auto placeKnob = [&](juce::Slider &s, juce::Label &l, int c, int r) {
    auto bounds =
        juce::Rectangle<int>(rightPanel.getX() + c * knobW,
                             rightPanel.getY() + r * knobH, knobW, knobH)
            .reduced(15);
    // Label at Bottom for knobs usually looks good, or Top.
    // User image implied labels below? "Labels are: Size..."
    // I'll put Label at BOTTOM.
    l.setBounds(bounds.removeFromBottom(24));
    s.setBounds(bounds);
  };

  // Row 1
  placeKnob(earlySizeKnob, earlySizeLbl, 0, 0);
  placeKnob(earlyCrossKnob, earlyCrossLbl, 1, 0);
  placeKnob(diffKnob, diffLbl, 2, 0);

  // Row 2
  placeKnob(modRateKnob, modRateLbl, 0, 1);
  placeKnob(modDepthKnob, modDepthLbl, 1, 1);
  placeKnob(earlySendKnob, earlySendLbl, 2, 1);
}
