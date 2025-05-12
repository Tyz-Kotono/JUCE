/*
  ==============================================================================

    RotarySliderWithLabels.h
    Created: 28 Apr 2025 11:10:45am
    Author:  tyzTang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


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
};

struct RotarySliderWithLabels : juce::Slider
{
public:
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix):
        juce::Slider
        (
            juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
            juce::Slider::TextEntryBoxPosition::NoTextBox
        ),
        param(&rap),
        suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }


    ~RotarySliderWithLabels() override
    {
        setLookAndFeel(nullptr);
    }

    struct LabelPos
    {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;

    void paint(juce::Graphics&) override;
    void resized() override;

    int getTextHeight() const { return 14; }
    juce::Rectangle<int> getSliderBounds() const;
    juce::String GetDisplayString() const;

private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotarySliderWithLabels)
};
