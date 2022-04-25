/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//make custom rotary sliders, generic template
struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider(): juce::Slider(SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};



//==============================================================================
/**
*/
class AmpsimAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AmpsimAudioProcessorEditor (AmpsimAudioProcessor&);
    ~AmpsimAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    AmpsimAudioProcessor& audioProcessor;
    
    CustomRotarySlider peakFreqSlider,
    peakGainSlider,
    peakQualitySlider,
    lowCutFreqSlider,
    highCutFreqSlider,
    levelOutput;
    
    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpsimAudioProcessorEditor)
};
