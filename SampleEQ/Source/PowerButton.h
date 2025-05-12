/*
  ==============================================================================

    PowerButton.h
    Created: 12 May 2025 7:01:04pm
    Author:  tyzTang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class AnalyzerButton : public juce::ToggleButton
{
public:
  juce::Path randomPath;
  void resized() override;
};

class PowerButton : public juce::ToggleButton
{
public:
   
};
