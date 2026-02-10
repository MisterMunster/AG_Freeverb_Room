#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "Parameters.h"

class AntigravPlateReverbAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    AntigravPlateReverbAudioProcessorEditor (AntigravPlateReverbAudioProcessor&);
    ~AntigravPlateReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;

private:
    AntigravPlateReverbAudioProcessor& audioProcessor;
    UI::DarkLookAndFeel darkLnF;

    // Primary Sliders (Vertical)
    juce::Slider mixSlider, predelaySlider, decaySlider, loCutSlider, hiCutSlider, depthSlider;
    juce::Label mixLabel, predelayLabel, decayLabel, loCutLabel, hiCutLabel, depthLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> mixAtt, predelayAtt, decayAtt, loCutAtt, hiCutAtt, depthAtt;

    // Mode Toggle
    juce::TextButton modeButton { "Early / Late" };
    bool showLate = false;

    // Knob Group (Early)
    juce::Slider earlySizeKnob, earlyCrossKnob, modRateKnob, modDepthKnob, earlySendKnob, diffKnob;
    juce::Label earlySizeLbl, earlyCrossLbl, modRateLbl, modDepthLbl, earlySendLbl, diffLbl;
    std::unique_ptr<SliderAttachment> eSizeAtt, eCrossAtt, mRateAtt, mDepthAtt, eSendAtt, diffAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AntigravPlateReverbAudioProcessorEditor)
};
