/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DspFaust.cpp"

//==============================================================================
/**
*/
class FaustSkeletonAudioProcessor  : public foleys::MagicProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
                             , private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    FaustSkeletonAudioProcessor();
    ~FaustSkeletonAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
   
    void parameterChanged (const juce::String& param, float value);

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    //Not needed with PGM
    //juce::AudioProcessorEditor* createEditor() override;
    //bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    //Not needed with PGM
    //void getStateInformation (juce::MemoryBlock& destData) override;
    //void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    std::unique_ptr<::DspFaust> faustDsp;
    std::unique_ptr<::MapUI> mapUI;
    
    float ** faustIO;
    
    int numParams;
    std::vector<std::string> paramNames {};

    juce::AudioProcessorValueTreeState treeState;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaustSkeletonAudioProcessor)
};
