#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AntigravPlateReverbAudioProcessor::AntigravPlateReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts (*this, nullptr, "Parameters", Params::createParameterLayout())
#endif
{
}

AntigravPlateReverbAudioProcessor::~AntigravPlateReverbAudioProcessor()
{
}

//==============================================================================
const juce::String AntigravPlateReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AntigravPlateReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AntigravPlateReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AntigravPlateReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AntigravPlateReverbAudioProcessor::getTailLengthSeconds() const
{
    return 3.0; // Longer tail for plate reverb
}

int AntigravPlateReverbAudioProcessor::getNumPrograms()
{
    return 3;
}

int AntigravPlateReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AntigravPlateReverbAudioProcessor::setCurrentProgram (int index)
{
    auto setParam = [&](const juce::String& id, float value) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
    };

    if (index == 0) // Bright Plate
    {
        setParam(Params::decay, 2.5f);
        setParam(Params::predelay, 10.0f);
        setParam(Params::earlySize, 80.0f);
        setParam(Params::diffusion, 1.0f);
        setParam(Params::hiCut, 14000.0f);
        setParam(Params::loCut, 60.0f);
        setParam(Params::modDepth, 35.0f);
        setParam(Params::earlySend, 0.5f);
    }
    else if (index == 1) // Warm Plate
    {
        setParam(Params::decay, 4.0f);
        setParam(Params::predelay, 25.0f);
        setParam(Params::earlySize, 120.0f);
        setParam(Params::diffusion, 0.9f);
        setParam(Params::hiCut, 6000.0f);
        setParam(Params::loCut, 80.0f);
        setParam(Params::modDepth, 25.0f);
        setParam(Params::earlySend, 0.4f);
    }
    else if (index == 2) // Ambient Plate
    {
        setParam(Params::decay, 8.0f);
        setParam(Params::predelay, 40.0f);
        setParam(Params::earlySize, 200.0f);
        setParam(Params::diffusion, 1.0f);
        setParam(Params::hiCut, 8000.0f);
        setParam(Params::loCut, 40.0f);
        setParam(Params::modDepth, 50.0f);
        setParam(Params::earlySend, 0.6f);
    }
}

const juce::String AntigravPlateReverbAudioProcessor::getProgramName (int index)
{
    if (index == 0) return "Bright Plate";
    if (index == 1) return "Warm Plate";
    if (index == 2) return "Ambient Plate";
    return {};
}

void AntigravPlateReverbAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void AntigravPlateReverbAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    preDelayL.prepare(sampleRate, 2000.0);
    preDelayR.prepare(sampleRate, 2000.0);
    earlyReflections.prepare(sampleRate);
    lateReverb.prepare(sampleRate);
}

void AntigravPlateReverbAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AntigravPlateReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

void AntigravPlateReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Get Parameters
    float mixVal = *apvts.getRawParameterValue(Params::mix) / 100.0f;
    float predelayMs = *apvts.getRawParameterValue(Params::predelay);
    float decayS = *apvts.getRawParameterValue(Params::decay);
    float loCutHz = *apvts.getRawParameterValue(Params::loCut);
    float hiCutHz = *apvts.getRawParameterValue(Params::hiCut);
    float modDepthVal = *apvts.getRawParameterValue(Params::modDepth) / 100.0f;

    // Early Params
    float earlySizeMs = *apvts.getRawParameterValue(Params::earlySize);
    float earlyCrossVal = *apvts.getRawParameterValue(Params::earlyCross);
    float diffusionVal = *apvts.getRawParameterValue(Params::diffusion);
    float earlySendVal = *apvts.getRawParameterValue(Params::earlySend);

    // Late Params
    float modRateVal = *apvts.getRawParameterValue(Params::modRate);
    float subModDepth = *apvts.getRawParameterValue(Params::modDepthSub);

    // Update DSP
    earlyReflections.setParameters(earlySizeMs, earlyCrossVal, diffusionVal);
    lateReverb.setParameters(decayS, subModDepth * modDepthVal, modRateVal, hiCutHz, loCutHz);

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    // Auxiliary buffers
    juce::AudioBuffer<float> preDelayBuf;
    preDelayBuf.setSize(2, numSamples);

    // Pre-Delay Processing
    for (int i = 0; i < numSamples; ++i)
    {
        preDelayL.push(left[i]);
        preDelayR.push(right[i]);

        preDelayBuf.setSample(0, i, preDelayL.read(predelayMs));
        preDelayBuf.setSample(1, i, preDelayR.read(predelayMs));
    }

    // Early Reflections
    juce::AudioBuffer<float> earlyBuf;
    earlyBuf.makeCopyOf(preDelayBuf);
    earlyReflections.processBlock(earlyBuf);

    // Late Reverb Input: PreDelayed + Early * Send
    juce::AudioBuffer<float> lateBuf;
    lateBuf.setSize(2, numSamples);

    auto* plL = preDelayBuf.getReadPointer(0);
    auto* plR = preDelayBuf.getReadPointer(1);
    auto* eL = earlyBuf.getReadPointer(0);
    auto* eR = earlyBuf.getReadPointer(1);
    auto* lL = lateBuf.getWritePointer(0);
    auto* lR = lateBuf.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        lL[i] = plL[i] + eL[i] * earlySendVal;
        lR[i] = plR[i] + eR[i] * earlySendVal;
    }

    lateReverb.processBlock(lateBuf);

    // Final Mix
    auto* lLat = lateBuf.getReadPointer(0);
    auto* rLat = lateBuf.getReadPointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        // Wet = Early + Late
        float wetL = eL[i] + lLat[i];
        float wetR = eR[i] + rLat[i];

        left[i] = dryL * (1.0f - mixVal) + wetL * mixVal;
        right[i] = dryR * (1.0f - mixVal) + wetR * mixVal;
    }
}

//==============================================================================
bool AntigravPlateReverbAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AntigravPlateReverbAudioProcessor::createEditor()
{
    return new AntigravPlateReverbAudioProcessorEditor (*this);
}

//==============================================================================
void AntigravPlateReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AntigravPlateReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AntigravPlateReverbAudioProcessor();
}
