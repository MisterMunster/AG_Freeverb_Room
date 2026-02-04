#pragma once

#include <JuceHeader.h>

namespace Params
{
    static const juce::String mix = "mix";
    static const juce::String predelay = "predelay";
    static const juce::String decay = "decay";
    static const juce::String loCut = "locut";
    static const juce::String hiCut = "hicut";
    static const juce::String modDepth = "mod_depth_main";

    static const juce::String earlySize = "early_size";
    static const juce::String earlyCross = "early_cross";
    static const juce::String modRate = "mod_rate";
    static const juce::String modDepthSub = "mod_depth_sub";
    static const juce::String diffusion = "diffusion";
    
    // Toggles/Mode
    static const juce::String mode = "mode"; // 0 = Early, 1 = Late (for UI, but maybe processing too?)
    // Actually the request implies both run or are toggled for *view*. 
    // "Early" and "Late" reflection modes (with Early as default...). 
    // Usually a reverb has both running. The toggle might be for the UI knobs.
    // But "Early Send" to late suggests they are connected.
    // I will assume both always run, and the UI toggles which set of fine-tune knobs are visible.
    // Though "Early Send" controls how much goes to Late.
    
    static const juce::String earlySend = "early_send"; // How much of Early goes to Late

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        auto makeParam = [&](const juce::String& id, const juce::String& name, float min, float max, float def, float skew = 1.0f)
        {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                id, name,
                juce::NormalisableRange<float>(min, max, 0.01f, skew),
                def));
        };

        makeParam(mix, "Mix", 0.0f, 100.0f, 100.0f);
        makeParam(predelay, "Predelay", 0.0f, 200.0f, 10.0f, 0.5f); // Skew for exp feel
        makeParam(decay, "Decay Time", 0.1f, 10.0f, 2.0f, 0.5f);
        makeParam(loCut, "Lo Cut", 20.0f, 500.0f, 20.0f, 0.5f);
        makeParam(hiCut, "Hi Cut", 1000.0f, 20000.0f, 6000.0f, 0.5f);
        makeParam(modDepth, "Mod Depth", 0.0f, 100.0f, 0.0f);

        makeParam(earlySize, "Early Size", 10.0f, 500.0f, 300.0f, 0.5f);
        makeParam(earlyCross, "Early Cross", 0.0f, 1.0f, 0.1f);
        makeParam(modRate, "Mod Rate", 0.1f, 5.0f, 0.5f, 0.5f);
        makeParam(modDepthSub, "Sub Mod Depth", 0.0f, 1.0f, 0.5f);
        makeParam(diffusion, "Diffusion", 0.0f, 1.0f, 1.0f);
        makeParam(earlySend, "Early Send", 0.0f, 1.0f, 0.0f); // Default 0 means parallel? Request says "Controls amount of early sent to late". Usually cascade.
        // If 0, late gets Dry input?
        // I'll implement logic: Late Input = Dry * (1-EarlySend) + Early * EarlySend ?
        // Or usually Late is fed by Early.
        // "Default: 0.00" suggests by default Late is fed by Dry directly (Parallel).

        return { params.begin(), params.end() };
    }
}
