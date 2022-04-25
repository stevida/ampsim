/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "my_convolution.h"

//==============================================================================
AmpsimAudioProcessorEditor::AmpsimAudioProcessorEditor (AmpsimAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

AmpsimAudioProcessorEditor::~AmpsimAudioProcessorEditor()
{
}

//==============================================================================
void AmpsimAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::green);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AmpsimAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

std::vector<juce::Component*> AmpsimAudioProcessorEditor::getComps(){
    std::vector<struct CustomRotarySlider> sliders;

    //returning a vect
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &levelOutput
    };

}
