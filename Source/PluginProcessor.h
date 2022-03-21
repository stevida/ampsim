/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//need to extract parameters from the audio processor value tree state
// implementing a struct
struct ChainSettings{
    float peakFreq{0}, peakGainInDecibels{0} , peakQuality{1.f};
    float lowCutFreq{0}, highCutFreq{0};
    int lowCutSlope{0}, highCutSlope{0};
};


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);


//==============================================================================
/**
*/
class AmpsimAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AmpsimAudioProcessor();
    ~AmpsimAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    //need to create createParameter Layout bc the APVT requires a list of the parametets for when you call it
    
    using APVT =juce::AudioProcessorValueTreeState::ParameterLayout;
    static APVT createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr,"Parameters",createParameterLayout()};

    //set up to processs mono audio, needs to process stereo so change,
    
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
    
    /**
                    mono chain: lowcut -> parametric -> highcut
     
     
     */
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain leftChain,rightChain;
    
    //to define the elements in the chain 
    enum ChainPosititions
    {
        lowCut,
        Peak,
        highCut,
    };
    
    


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpsimAudioProcessor)
};
