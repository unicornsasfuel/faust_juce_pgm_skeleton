/*
  ==============================================================================

   This file implements a JUCE effect plugin that expects FaustAPI generated
   DSP files, such as those exported from the FAUST WebIDE using the
   "source -> juce" option.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include <JuceHeader.h>


juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    std::unique_ptr<::DspFaust> tempDsp = std::make_unique<::DspFaust>(); 

    int numParams = tempDsp->getParamsCount();

    for (int i = 0; i < numParams; i++) {
        std::string paramName = tempDsp->getParamAddress(i);
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(paramName, 1),
            paramName,
            juce::NormalisableRange<float>(
                tempDsp->getParamMin(paramName.c_str()),
                tempDsp->getParamMax(paramName.c_str())
            ),
            tempDsp->getParamInit(paramName.c_str()))
        );
    }

    
    return layout;
}

//==============================================================================
FaustSkeletonAudioProcessor::FaustSkeletonAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : foleys::MagicProcessor (BusesProperties() 
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
     ),
    treeState (*this, nullptr, "PARAMETERS", createParameterLayout())
#endif

{
    faustDsp = std::make_unique<::DspFaust>();

    //add parameter listeners
    numParams = faustDsp->getParamsCount();

    for (int i = 0; i < numParams; i++) {
        std::string paramName = faustDsp->getParamAddress(i);
        paramNames.push_back(paramName);
        treeState.addParameterListener (paramName, this);
    }
    
#ifdef MAGIC_LOAD_BINARY    
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
#endif

    faustDsp->start();
}

FaustSkeletonAudioProcessor::~FaustSkeletonAudioProcessor()
{
    faustDsp->stop();
}

//==============================================================================
const juce::String FaustSkeletonAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FaustSkeletonAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FaustSkeletonAudioProcessor::producesMidi() const
{
    return false;
}

bool FaustSkeletonAudioProcessor::isMidiEffect() const
{
    return false;
}

double FaustSkeletonAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FaustSkeletonAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FaustSkeletonAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FaustSkeletonAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FaustSkeletonAudioProcessor::getProgramName (int index)
{
    return {};
}

void FaustSkeletonAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void FaustSkeletonAudioProcessor::parameterChanged (const juce::String& param, float value)
{

    faustDsp->setParamValue(param.toRawUTF8(), value);

}

//==============================================================================
void FaustSkeletonAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    // Report latency to host
    // by samples
    setLatencySamples(0);
    // by seconds
    //setLatencySamples(meta.fLatencySec * sampleRate);
    
}

void FaustSkeletonAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FaustSkeletonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FaustSkeletonAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaustSkeletonAudioProcessor();
}
