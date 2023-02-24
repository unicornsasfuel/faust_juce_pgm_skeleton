
#include "PluginProcessor.h"
#include <JuceHeader.h>

// Uncomment the #define directive below when your PGM XML file is ready
// Add it to the project files with the name "magic.xml"

//#define MAGIC_LOAD_BINARY 1

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : foleys::MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       treeState (*this, nullptr)
#endif
{
    faustDsp = std::make_unique<::DspFaust>();
    
    //Iterate through UI parameters and add processor parameters / parameter listeners
    numParams = faustDsp->getParamsCount();

    for (int i = 0; i < numParams; i++) {
        std::string paramName = faustDsp->getParamAddress(i);
        paramNames.push_back(paramName);
        addParameter(new juce::AudioParameterFloat(
            juce::ParameterID(paramName, 1),
            paramName,
            juce::NormalisableRange<float>(
                faustDsp->getParamMin(i),
                faustDsp->getParamMax(i)
                ),
            faustDsp->getParamInit(i))
        );
        treeState.addParameterListener (paramName, this);
    }
    
#ifdef MAGIC_LOAD_BINARY    
    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
#endif

    faustDsp->start();

}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
    faustDsp->stop();
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void NewProjectAudioProcessor::parameterChanged (const juce::String& param, float value)
{

    faustDsp->setParamValue(param.toRawUTF8(), value);

}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Parse Faust metadata to automatically set latency
    juce::var parsedJson;
    std::string jsonMeta = faustDsp->getJSONMeta();
    if (!juce::JSON::parse(jsonMeta, parsedJson).failed()) {
        auto meta = parsedJson["meta"];
        int metaSize = meta.size();

        for (int i = 0; i < metaSize; i++) {
            int foo = int(meta[i]["latency_samples"]);
            if (meta[i]["latency_samples"]) {
                setLatencySamples(int(meta[i]["latency_samples"]));
            }
            else if (meta[i]["latency_sec"]) {
                setLatencySamples(int(meta[i]["latency_sec"]) * sampleRate);
            }
        }
    }

}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        //TODO: this timestamp may not be in usec, need to read further
        const int time = message.getTimeStamp();
        int channel = message.getChannel();
        const juce::uint8* rawmidi = message.getRawData();
        int type = rawmidi[0] & 0xf0;
        int count = message.getMessageLengthFromFirstByte(rawmidi[0]);
        //DEBUG: Log MIDI messages
        juce::Logger::writeToLog(message.getDescription());

        int data1 = NULL;
        int data2 = NULL;
        if (count > 1) {
            data1 = rawmidi[1];
            if (count > 2) {
                data2 = rawmidi[2];
            }
        }
        faustDsp->propagateMidi(count, time, type, channel, data1, data2);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
