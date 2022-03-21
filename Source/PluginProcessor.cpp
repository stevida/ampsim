/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmpsimAudioProcessor::AmpsimAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

AmpsimAudioProcessor::~AmpsimAudioProcessor()
{
}

//==============================================================================
const juce::String AmpsimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmpsimAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AmpsimAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AmpsimAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AmpsimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmpsimAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AmpsimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmpsimAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AmpsimAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmpsimAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AmpsimAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //passing process spec object to the chain, a struct that holds info for the dsp for initialization
    //this is where the DSP initilization occured
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    auto chainSettings = getChainSettings(apvts);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));//needs to be in gain units, not decibels
    *leftChain.get<ChainPosititions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosititions::Peak>().coefficients = *peakCoefficients;
    
    
    
    //NEEDS AN INDEX, CREATE AN ENUMRATOR TO KNOW WHAT EACH ELEMENT IN THE ARRAY IS
    //define an enumerator to define each links position in the chain, or each element in the array
    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void AmpsimAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmpsimAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AmpsimAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    //taken from the prepareToPlay function
    auto chainSettings = getChainSettings(apvts);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));//needs to be in gain units, not decibels
    *leftChain.get<ChainPosititions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosititions::Peak>().coefficients = *peakCoefficients;
    
    //need to extract the left and right channels from the block that contains the buffer
    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    
}

//==============================================================================
bool AmpsimAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AmpsimAudioProcessor::createEditor()
{
    
    //return new AmpsimAudioProcessorEditor (*this); this returns the default hello world module, we can use the generic one
    return new juce::GenericAudioProcessorEditor(*this);

}

//==============================================================================
void AmpsimAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AmpsimAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//helper function to get the parameters of the audiotreevaluestate
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts){
    ChainSettings settings;
    
    /**
            taking the values from the audiotreevaluestate and putting them into the settings struct
     */
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load(); //doesn't return a     normalized value
    settings.highCutFreq= apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope =apvts.getRawParameterValue("LowCut Slope")->load();
    settings.highCutSlope =apvts.getRawParameterValue("HighCut Slope")->load();
    
    return settings;
    
    
}
juce::AudioProcessorValueTreeState::ParameterLayout AmpsimAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                            "LowCut Freq",
                                                            juce::NormalisableRange<float>(20.f,20000.f, 1.f,0.5f),
                                                            20.f));
    
    
    /**
                order of add variables
                layout.add(<#std::unique_ptr<Items> items...#>)
                layout.add(std::make_unique<juce::AudioParameterFloat(<#const String &parameterID#>, <#const String &parameterName#>, juce::NormalisableRange<float>, <#float defaultValue#>)> )
       
                NormalisableRange (ValueType rangeStart,
                                   ValueType rangeEnd,
                                   ValueType intervalValue,
                                   ValueType skewFactor, // alters the way values are distrubuted across the range so that larger or smaller values are given a larger proportion of the available space
                                   bool useSymmetricSkew = false)
               **/
       layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                              "HighCut Freq",
                                                              juce::NormalisableRange<float>(20.f,20000.f, 1.f,0.5f),
                                                              20000.f));
       
       layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                              "Peak Freq",
                                                              juce::NormalisableRange<float>(20.f,20000.f, 1.f,0.5f),
                                                              750.f));
      //*  peak gain will be expressed in decibels, using values from -24 to 24, w/ stepsize of 0.5, starting at 0 so there is not gain added at the beginning
       layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                              "Peak Gain",
                                                              juce::NormalisableRange<float>(-24.f,24.f, 0.5f,1.f),
                                                              0.0f));
    
       //quality control, narrow q is accomplished with high q value, wide q is accomplished with low q value
       layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
                                                              "Peak Quality",
                                                              juce::NormalisableRange<float>(0.1f,10.f, 0.05f,1.f),
                                                              1.f));
    
    
    //represents the cutoff slope of each  filter types, usually between 12 and 48 at increments of 12
    juce::StringArray stringArray;
        for(int i = 0; i < 4;++i){
            juce::String str;
            str << (12 + i*12); // gives us 12, 24, 36 and 48 db/Oct
            str<< " db/Oct";
            stringArray.add(str);
        }
        
        // choice functions allows for a drop down menu, just for representing them
        layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope","LowCut Slope",stringArray,0));
        layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope","HighCut Slope",stringArray,0));
                   
        
        
        return layout;
        
    
    
}




//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    
    return new AmpsimAudioProcessor();

}
