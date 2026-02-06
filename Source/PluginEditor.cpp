#include "PluginEditor.h"

//==============================================================================
AntigravReverbAudioProcessorEditor::AntigravReverbAudioProcessorEditor (AntigravReverbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&darkLnF);
    bgImage = juce::ImageCache::getFromMemory(BinaryData::AGreverb_jpg, BinaryData::AGreverb_jpgSize);

    auto setupSlider = [this](juce::Slider& s, juce::Label& l, std::unique_ptr<SliderAttachment>& att, const juce::String& pID, const juce::String& name, bool vertical)
    {
        s.setSliderStyle(vertical ? juce::Slider::LinearVertical : juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(s);
        
        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
        
        att = std::make_unique<SliderAttachment>(audioProcessor.apvts, pID, s);
    };

    // Primary
    setupSlider(mixSlider, mixLabel, mixAtt, Params::mix, "Mix", true);
    setupSlider(predelaySlider, predelayLabel, predelayAtt, Params::predelay, "PreDelay", true);
    setupSlider(decaySlider, decayLabel, decayAtt, Params::decay, "Decay", true);
    setupSlider(loCutSlider, loCutLabel, loCutAtt, Params::loCut, "Lo Cut", true);
    setupSlider(hiCutSlider, hiCutLabel, hiCutAtt, Params::hiCut, "Hi Cut", true);
    setupSlider(depthSlider, depthLabel, depthAtt, Params::modDepth, "Depth", true);

    // Early Knobs
    setupSlider(earlySizeKnob, earlySizeLbl, eSizeAtt, Params::earlySize, "Size", false);
    setupSlider(earlyCrossKnob, earlyCrossLbl, eCrossAtt, Params::earlyCross, "Cross", false);
    setupSlider(modRateKnob, modRateLbl, mRateAtt, Params::modRate, "Rate", false);
    setupSlider(modDepthKnob, modDepthLbl, mDepthAtt, Params::modDepthSub, "Mod Lv", false);
    setupSlider(earlySendKnob, earlySendLbl, eSendAtt, Params::earlySend, "Send", false);
    setupSlider(diffKnob, diffLbl, diffAtt, Params::diffusion, "Diff", false);

    // Toggle
    addAndMakeVisible(modeButton);
    modeButton.setClickingTogglesState(true);
    modeButton.addListener(this);
    modeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    modeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkred); // "Highlighted in red"
    
    // Initial State
    showLate = false;
    modeButton.setToggleState(false, juce::dontSendNotification); // Default Early
    buttonClicked(&modeButton); // Trigger visibility update

    setSize (800, 500);
}

AntigravReverbAudioProcessorEditor::~AntigravReverbAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void AntigravReverbAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &modeButton)
    {
        showLate = modeButton.getToggleState();
        bool earlyVisible = !showLate;
        
        // Show/Hide Early Knobs
        earlySizeKnob.setVisible(earlyVisible); earlySizeLbl.setVisible(earlyVisible);
        earlyCrossKnob.setVisible(earlyVisible); earlyCrossLbl.setVisible(earlyVisible);
        earlySendKnob.setVisible(earlyVisible); earlySendLbl.setVisible(earlyVisible);
        diffKnob.setVisible(earlyVisible); diffLbl.setVisible(earlyVisible);
        // Mod Rate/Depth shared?
        modRateKnob.setVisible(true); modRateLbl.setVisible(true); // Always visible or toggled context?
        modDepthKnob.setVisible(true); modDepthLbl.setVisible(true);
        
        // If Late had specific knobs, would show here.
        // Since Late shares, we just keep Mod knobs.
        
        modeButton.setButtonText(showLate ? "Mode: LATE" : "Mode: EARLY");
    }
}

//==============================================================================
void AntigravReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    if (bgImage.isValid())
        g.drawImage(bgImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    
    // Divide Left/Right panels visually
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    auto leftPanel = getLocalBounds().removeFromLeft((int)(getWidth() * 0.4f));
    g.fillRect(leftPanel);
    
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Antigrav Reverb", leftPanel.removeFromTop(40), juce::Justification::centred, 1);
}

void AntigravReverbAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto leftPanel = area.removeFromLeft((int)(getWidth() * 0.4f));
    auto rightPanel = area;
    
    // Left Panel Layout: 6 sliders
    leftPanel.removeFromTop(40); // Title
    int w = leftPanel.getWidth() / 3;
    int h = leftPanel.getHeight() / 2;
    
    auto placeSlider = [&](juce::Slider& s, juce::Label& l, int x, int y)
    {
        auto r = leftPanel.getX() + x * w;
        auto ry = leftPanel.getY() + y * h;
        auto cell = juce::Rectangle<int>(r, ry, w, h).reduced(10);
        
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
    };
    
    placeSlider(mixSlider, mixLabel, 0, 0);
    placeSlider(predelaySlider, predelayLabel, 1, 0);
    placeSlider(decaySlider, decayLabel, 2, 0);
    placeSlider(loCutSlider, loCutLabel, 0, 1);
    placeSlider(hiCutSlider, hiCutLabel, 1, 1);
    placeSlider(depthSlider, depthLabel, 2, 1);
    
    // Right Panel
    auto topBar = rightPanel.removeFromTop(50);
    modeButton.setBounds(topBar.removeFromRight(150).reduced(10));
    
    // Grid for Knobs
    int kw = rightPanel.getWidth() / 3;
    int kh = rightPanel.getHeight() / 2;
    
    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int x, int y)
    {
        auto r = rightPanel.getX() + x * kw;
        auto ry = rightPanel.getY() + y * kh;
        auto cell = juce::Rectangle<int>(r, ry, kw, kh).reduced(20);
        l.setBounds(cell.removeFromTop(20));
        s.setBounds(cell);
    };
    
    if (!showLate)
    {
        placeKnob(earlySizeKnob, earlySizeLbl, 0, 0);
        placeKnob(earlyCrossKnob, earlyCrossLbl, 1, 0);
        placeKnob(diffKnob, diffLbl, 2, 0);
        placeKnob(modRateKnob, modRateLbl, 0, 1);
        placeKnob(modDepthKnob, modDepthLbl, 1, 1);
        placeKnob(earlySendKnob, earlySendLbl, 2, 1);
    }
    else
    {
        // Late Layout
        placeKnob(modRateKnob, modRateLbl, 0, 0);
        placeKnob(modDepthKnob, modDepthLbl, 1, 0);
    }
}
