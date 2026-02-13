#pragma once

#include <JuceHeader.h>

namespace Params
{
    // Same parameter IDs as the room version for control compatibility
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

    static const juce::String mode = "mode";
    static const juce::String earlySend = "early_send";

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

        // Primary controls - same names, plate-tuned defaults
        makeParam(mix, "Mix", 0.0f, 100.0f, 100.0f);
        makeParam(predelay, "Predelay", 0.0f, 200.0f, 20.0f, 0.5f);
        // Plate reverbs can sustain longer than rooms
        makeParam(decay, "Decay Time", 0.1f, 15.0f, 3.0f, 0.5f);
        makeParam(loCut, "Lo Cut", 20.0f, 500.0f, 40.0f, 0.5f);
        // Plates are naturally brighter - higher default hi-cut
        makeParam(hiCut, "Hi Cut", 1000.0f, 20000.0f, 10000.0f, 0.5f);
        // Default modulation on for lush plate character
        makeParam(modDepth, "Mod Depth", 0.0f, 100.0f, 30.0f);

        // Early reflection controls - plate-tuned
        // Plate "size" is smaller (the physical plate is compact)
        makeParam(earlySize, "Early Size", 5.0f, 250.0f, 100.0f, 0.5f);
        // Higher cross-feed default (plate is a single vibrating surface)
        makeParam(earlyCross, "Early Cross", 0.0f, 1.0f, 0.3f);
        makeParam(modRate, "Mod Rate", 0.1f, 5.0f, 0.8f, 0.5f);
        makeParam(modDepthSub, "Sub Mod Depth", 0.0f, 1.0f, 0.6f);
        // High diffusion default for plate density
        makeParam(diffusion, "Diffusion", 0.0f, 1.0f, 1.0f);
        // Plates typically cascade early into late
        makeParam(earlySend, "Early Send", 0.0f, 1.0f, 0.4f);

        return { params.begin(), params.end() };
    }
}
