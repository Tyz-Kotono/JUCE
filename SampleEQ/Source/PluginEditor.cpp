/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SampleEQAudioProcessorEditor::SampleEQAudioProcessorEditor(SampleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      responseCurveComponent(audioProcessor),
      peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
      peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
      peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
      lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
      lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
      highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
      highCutSlopeSlider(*audioProcessor.apvts.getParameter("High Slope"), "dB/Oct"),

      peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
      peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
      peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
      lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
      lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
      highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
      highCutSlopeSliderAttachment(audioProcessor.apvts, "High Slope", highCutSlopeSlider),

      lowCutBypassButtonAttachment(audioProcessor.apvts, lowCutBypass, lowCutBypassButton),
      peakBypassButtonAttachment(audioProcessor.apvts, peakByPass, peakBypassButton),
      highCutBypassButtonAttachment(audioProcessor.apvts, highCutBypass, highCutBypassButton),
      analyzerEnableButtonAttachment(audioProcessor.apvts, analyzerByPass, analyzerEnableButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});
    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});


    for (auto* comp : GetComps())
    {
        addAndMakeVisible(comp);
    }

    peakBypassButton.setLookAndFeel(&lnf);
    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);
    analyzerEnableButton.setLookAndFeel(&lnf);


    auto safePtr = juce::Component::SafePointer<SampleEQAudioProcessorEditor>(this);
    peakBypassButton.onClick = [safePtr]
    {

        if(auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->peakBypassButton.getToggleState();

            comp->peakFreqSlider.setEnabled(!bypassed);
            comp->peakGainSlider.setEnabled(!bypassed);
            comp->peakQualitySlider.setEnabled(!bypassed);
            
        }
    };

    lowCutBypassButton.onClick = [safePtr]
    {

        if(auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->lowCutBypassButton.getToggleState();

            comp->lowCutFreqSlider.setEnabled(!bypassed);
            comp->lowCutSlopeSlider.setEnabled(!bypassed);
        }
    };

    highCutBypassButton.onClick = [safePtr]
    {

        if(auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->highCutBypassButton.getToggleState();

            comp->highCutFreqSlider.setEnabled(!bypassed);
            comp->highCutSlopeSlider.setEnabled(!bypassed);
        }
    };
    analyzerEnableButton.onClick = [safePtr]
    {

        if(auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->analyzerEnableButton.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnableemet(!bypassed);
        }
    };


    
    setSize(600, 400);
}

SampleEQAudioProcessorEditor::~SampleEQAudioProcessorEditor()
{
    peakBypassButton.setLookAndFeel(nullptr);
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    analyzerEnableButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SampleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;
    // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colours::black);
}

void SampleEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto analyzerEnabledArea = bounds.removeFromTop(25);
    analyzerEnabledArea.setWidth(100);
    analyzerEnabledArea.setX(5);
    analyzerEnabledArea.removeFromTop(2);
    analyzerEnableButton.setBounds(analyzerEnabledArea);

    bounds.removeFromTop(5);

    
    float hRatio = 0.33f; // JUCE_LIVE_CONSTANT(33)/100.0f;

    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(5);
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.3f);
    // 66 -> 0.5 = 0.33
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5f);


    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    // 66 -> 0.5 = 0.33
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}
