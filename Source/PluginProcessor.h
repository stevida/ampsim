/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//need to extract parameters from the audio processor value tree state
// implementing a struct

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings{
    float peakFreq{0}, peakGainInDecibels{0} , peakQuality{1.f};
    float lowCutFreq{0}, highCutFreq{0};
    
    Slope lowCutSlope{Slope::Slope_12}, highCutSlope{Slope::Slope_12};
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
    
    
    //need to create createParameter Layout bc the APVT requires a list of the parameters for when you call it
    
    using APVT = juce::AudioProcessorValueTreeState::ParameterLayout;
    static APVT createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr,"Parameters",createParameterLayout()};

    //set up to processs mono audio, needs to process stereo so change,
    
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter,Filter,Filter,Filter>;
    /** mono chain: lowcut -> parametric -> highcut*/
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain leftChain,rightChain;
    
    //to define the elements in the chain 
    enum ChainPosititions
    {
        lowCut,
        Peak,
        highCut,
    };
    
    void updatePeakFilter (const ChainSettings& chainSettings);
    
    /** alias used for the coefficient functions used*/
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    
    
    /**
        REFACTORED THIS FUNCTION TO MAKE THE CODE CLEANER
     
     */
    
    
    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients){
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }

    
    /**using a template so that we can reuse code for each filter, not just the lowpass, instead of having to explicitly type it out for each channel in each filter*/
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& chain,
                         const CoefficientType& coefficient,
                         const Slope& slope)
    {
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        chain.template setBypassed<2>(true);
        chain.template setBypassed<3>(true);
    
        
        switch (slope) {
            case Slope_48:
            {
                update<3>(chain,coefficient);
            }
            case Slope_36:
            {
                update<2>(chain,coefficient);
            }
            case Slope_24:
            {
                update<1>(chain,coefficient);
                break;
            }
            case Slope_12:
            {
                update<0>(chain,coefficient);
            }
        }
                
                
    }

                                                
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpsimAudioProcessor)
};
