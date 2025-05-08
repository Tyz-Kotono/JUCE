/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once
#include "ResponseCurveComponent.h"


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

    RotarySliderWithLabels
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
            &responseCurveComponent,

            &lowCutBypassButton,
            &peakBypassButton,
            &highCutBypassButton,
            &analyzerEnableButton,
        };
    }

    juce::ToggleButton lowCutBypassButton,peakBypassButton,highCutBypassButton,analyzerEnableButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEQAudioProcessorEditor)
};
