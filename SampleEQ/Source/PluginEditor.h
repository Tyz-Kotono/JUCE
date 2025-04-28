/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
    }
};

struct ResponseCurveComponent : juce::Component,
                                juce::AudioProcessorParameter::Listener,
                                juce::Timer
{
    ResponseCurveComponent(SampleEQAudioProcessor&);
    ~ResponseCurveComponent() override;

    //Call back
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    void timerCallback() override;

    void paint(juce::Graphics&) override;

private:
    SampleEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{false};

    MonoChain monoChain;
};

//==============================================================================
/**
*/
class SampleEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SampleEQAudioProcessorEditor(SampleEQAudioProcessor&);
    ~SampleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SampleEQAudioProcessor& audioProcessor;

    juce::Atomic<bool> parametersChanged{false};

    // MonoChain monoChain;
    ResponseCurveComponent responseCurveComponent;

    CustomRotarySlider
        peakFreqSlider,
        peakGainSlider,
        peakQualitySlider,
        lowCutFreqSlider,
        lowCutSlopeSlider,
        highCutFreqSlider,
        highCutSlopeSlider;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment
        peakFreqSliderAttachment,
        peakGainSliderAttachment,
        peakQualitySliderAttachment,
        lowCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutFreqSliderAttachment,
        highCutSlopeSliderAttachment;

    std::vector<juce::Component*> GetComps()
    {
        return
        {
            &peakFreqSlider,
            &peakGainSlider,
            &peakQualitySlider,
            &lowCutFreqSlider,
            &lowCutSlopeSlider,
            &highCutFreqSlider,
            &highCutSlopeSlider,
            &responseCurveComponent
        };
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEQAudioProcessorEditor)
};
