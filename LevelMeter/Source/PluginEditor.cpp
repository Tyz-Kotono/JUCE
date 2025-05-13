/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LevelMeterAudioProcessorEditor::LevelMeterAudioProcessorEditor(LevelMeterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
      // leftSliderAttachment(p.getApvts(), "left", leftSlider),
      // rightSliderAttachment(p.getApvts(), "right", rightSlider)
{
    addAndMakeVisible(horizontalMeterL);
    addAndMakeVisible(horizontalMeterR);

    setSize(400, 300);

    //Time Call back 1000.0f/24
    startTimerHz(24);
}

LevelMeterAudioProcessorEditor::~LevelMeterAudioProcessorEditor()
{
}

void LevelMeterAudioProcessorEditor::timerCallback()
{
    const auto leftGain = audioProcessor.getRmsLevel(0);
    const auto rightGain = audioProcessor.getRmsLevel(1);
    
    horizontalMeterL.setLevel(leftGain);
    horizontalMeterL.repaint();

    horizontalMeterR.setLevel(rightGain);
    horizontalMeterR.repaint();
}

//==============================================================================
void LevelMeterAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::darkgrey);
}

void LevelMeterAudioProcessorEditor::resized()
{
    // const auto container = getBounds().reduced(20);
    // auto bounds = container;
    //
    //
    // auto horizontalMeterBounds = bounds.removeFromTop(container.proportionOfHeight(0.1f)).reduced(5);
    // horizontalMeterL.setBounds(horizontalMeterBounds.removeFromTop(horizontalMeterBounds.proportionOfHeight(0.5f)).reduced(5));
    // horizontalMeterR.setBounds(horizontalMeterBounds.reduced(5));
    horizontalMeterL.setBounds(100,100,200,15);
    horizontalMeterR.setBounds(100,120,200,15);
}
