
/*
  ==============================================================================

    my_convolution.h
    Author:  Steven Vidal

  ==============================================================================
*/

#pragma once

template <typename Type>
class CabSimulator
{
public:
    CabSimulator()
    {
        auto dir = juce::File::getCurrentWorkingDirectory();
        
        int numTries = 0;
        
        while(!dir.getChildFile("project_resources").exists() && numTries++< 15) {
            dir = dir.getParentDirectory();
        }
        
        auto& convolution = processorChain.template get<convolutionIndex>();
        
        convolution.loadImpulseResponse     (dir.getChildFile("/Users/stevenvidal/Programming/CAPSTONE/ampsim/project_resources/guitar_amp.wav"),
                                          juce::dsp::Convolution::Stereo::yes,
                                          juce::dsp::Convolution::Trim::no,
                                          1024);
        
        
        
        
        
    }

    void prepare (const juce::dsp::ProcessSpec& spec) {
        
        /** post convolution filter
         */
        auto& post_filter = processorChain.template get<postConvolutionFilter>();
        post_filter.state = FilterCoefs::makeNotch(spec.sampleRate,1300.0f,0.1f);
        processorChain.prepare(spec);
    }
    
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept {
        
        
        processorChain.process(context);
        
    }
    void reset() noexcept {
        processorChain.reset();
    }
    
    
private:
    enum
    {
        convolutionIndex,
        postConvolutionFilter
    };
    /**name space used to make the filter calls within the DSP class easier to type out*/
    
    using Filter = juce::dsp::IIR::Filter<Type>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<Type>; // converts mono filters into a multi-channel version
    
    /*
     this is the  decleration of the  convolution chain object
     */
    juce::dsp::ProcessorChain<juce::dsp::Convolution,
    juce::dsp::ProcessorDuplicator<Filter,FilterCoefs>> processorChain;
    
    
};

template <typename Type>
class Distortion
{
public:
    //constructor of the distortion class
    //==============================================================================
    Distortion() {
        auto& waveshaper = processorChain.template get<waveshaperIndex>();//
        waveshaper.functionToUse = [] (Type x)
                                        {
                                            return x/(std::abs(x) + 1);
                                            //return std::tanh(x)//hyperbolic tangent transfer function
                                        };

        /*
         setting the gain for pre and post within the constructor
         */
        auto& preGain = processorChain.template get<preGainIndex>();
        preGain.setGainDecibels (40.0f); //boost
        
        auto& postGain = processorChain.template get<postGainIndex>();
        postGain.setGainDecibels(20.0f); // more of a boost?
        


    }

    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        auto& pre_filter = processorChain.template get<preFilterIndex>(); // reference to the filter processor
        //pre_filter.state = FilterCoefs::makeFirstOrderHighPass(spec.sampleRate, 1000.0f); //original,

        pre_filter.state =  FilterCoefs::makeFirstOrderLowPass(spec.sampleRate, 15000.0f);


        auto& post_filter = processorChain.template get<postFilterIndex>();
        post_filter.state = FilterCoefs::makeFirstOrderHighPass(spec.sampleRate, 400.0f);


        processorChain.prepare(spec);

    }

    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        processorChain.process(context);
    
    }

    //==============================================================================
    void reset() noexcept {
        processorChain.reset();
    }

private:
    //==============================================================================
    enum
    {
        /**
         each one of these elements is in the distortion clas chain
         preFilterIndex -> preGainIndex -> waveshaperIndex -> postFilterIndex -> postGainIndex
         
         */
        preFilterIndex,
        preGainIndex,
        waveshaperIndex,
        postFilterIndex,
        postGainIndex
    };
    
    //low ends kind of muddy, add a high pass filter to help out with that
    
    using Filter = juce::dsp::IIR::Filter<Type>;
    using FilterCoefs = juce::dsp::IIR::Coefficients<Type>; // converts mono filters into a multi-channel version
    
    
    /*
     SIGNAL CHAIN FOR THE DISTORTION CLASS
     */
    juce::dsp::ProcessorChain<juce::dsp::ProcessorDuplicator<Filter,FilterCoefs>,
    juce::dsp::Gain<Type>,
    juce::dsp::WaveShaper<Type>,
    juce::dsp::ProcessorDuplicator<Filter,FilterCoefs>,
    juce::dsp::Gain<Type>> processorChain; // <- this is the name of the object holding the signal chain within  the Distortion  class
    
};


class AudioEngine
{
public:
    
    AudioEngine(){
        
    }

    void prepare(const juce::dsp::ProcessSpec& spec) noexcept {
        fxChain.prepare(spec);
    }
   
    void process_convolution( juce::dsp::ProcessContextReplacing<float> context){
        fxChain.process(context);
    }
    
    
private:
    enum
    {
        distortionIndex,
        cabSimulatorIndex,
        postConvolutionEQ,
    };

    juce::dsp::ProcessorChain<Distortion<float>,CabSimulator<float>> fxChain;

};


