#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AntigravReverbAudioProcessor::AntigravReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts (*this, nullptr, "Parameters", Params::createParameterLayout())
#endif
{
}

AntigravReverbAudioProcessor::~AntigravReverbAudioProcessor()
{
}

//==============================================================================
const juce::String AntigravReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AntigravReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AntigravReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AntigravReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AntigravReverbAudioProcessor::getTailLengthSeconds() const
{
    return 2.0;
}

int AntigravReverbAudioProcessor::getNumPrograms()
{
    return 3; // Small, Medium, Large
}

int AntigravReverbAudioProcessor::getCurrentProgram()
{
    return 0; // Not tracking actual state index effectively without extra logic, keeping simple
}

void AntigravReverbAudioProcessor::setCurrentProgram (int index)
{
    // Helper to set param
    auto setParam = [&](const juce::String& id, float value) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
    };

    if (index == 0) // Small Room
    {
        setParam(Params::decay, 0.8f);
        setParam(Params::predelay, 5.0f);
        setParam(Params::earlySize, 60.0f);
        setParam(Params::diffusion, 0.8f);
        setParam(Params::hiCut, 8000.0f);
    }
    else if (index == 1) // Medium Room
    {
        setParam(Params::decay, 1.8f);
        setParam(Params::predelay, 15.0f);
        setParam(Params::earlySize, 180.0f);
        setParam(Params::diffusion, 1.0f);
        setParam(Params::hiCut, 6000.0f);
    }
    else if (index == 2) // Large Room
    {
        setParam(Params::decay, 4.0f);
        setParam(Params::predelay, 30.0f);
        setParam(Params::earlySize, 400.0f);
        setParam(Params::diffusion, 0.8f);
        setParam(Params::hiCut, 4000.0f);
    }
}

const juce::String AntigravReverbAudioProcessor::getProgramName (int index)
{
    if (index == 0) return "Small Room";
    if (index == 1) return "Medium Room";
    if (index == 2) return "Large Room";
    return {};
}

void AntigravReverbAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void AntigravReverbAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    preDelayL.prepare(sampleRate, 2000.0);
    preDelayR.prepare(sampleRate, 2000.0);
    earlyReflections.prepare(sampleRate);
    lateReverb.prepare(sampleRate);
}

void AntigravReverbAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AntigravReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AntigravReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
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
        // Push inputs
        preDelayL.push(left[i]);
        preDelayR.push(right[i]);
        
        preDelayBuf.setSample(0, i, preDelayL.read(predelayMs));
        preDelayBuf.setSample(1, i, preDelayR.read(predelayMs));
    }
    
    // Early Reflections
    // Input is PreDelayed signal
    juce::AudioBuffer<float> earlyBuf;
    earlyBuf.makeCopyOf(preDelayBuf);
    earlyReflections.processBlock(earlyBuf);
    
    // Late Reverb Input Logic
    // LateInput = PreDelayed * (1-EarlySend)? Or just PreDelayed + Early?
    // Let's go with: LateIn = PreDelayed + Early * Send.
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
bool AntigravReverbAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AntigravReverbAudioProcessor::createEditor()
{
    return new AntigravReverbAudioProcessorEditor (*this);
}

//==============================================================================
void AntigravReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AntigravReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AntigravReverbAudioProcessor();
}
