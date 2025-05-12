/*
  ==============================================================================

    LookAndFeel.h
    Created: 12 May 2025 7:07:33pm
    Author:  tyzTang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class AnalyzerButton;

struct LookAndFeel : juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& togglebutton,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;


    void DrawPowerButton(juce::Graphics& g,
                         juce::ToggleButton& togglebutton);
    void DrawAnalyzerButton(juce::Graphics& g,
                         AnalyzerButton& togglebutton);
};
