/*
  ==============================================================================

    RotarySliderWithLabels.cpp
    Created: 28 Apr 2025 11:10:45am
    Author:  tyzTang

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"

//==============================================================================

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId)); // clear the background

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1); // draw an outline around the component

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("RotarySliderWithLabels", getLocalBounds(),
               juce::Justification::centred, true); // draw some placeholder text
}

void RotarySliderWithLabels::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

