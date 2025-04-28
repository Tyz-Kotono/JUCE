/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"


ResponseCurveComponent::ResponseCurveComponent(SampleEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& parmas = audioProcessor.getParameters();

    for (auto* param : parmas)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& parmas = audioProcessor.getParameters();

    for (auto* param : parmas)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        // Update momo chain

        DBG("Params changer");
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        UpdateCoefficients(monoChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);

        auto lowCutCoefficients = makeLowCutFilters(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilters(chainSettings, audioProcessor.getSampleRate());

        UpdateCutFilter(monoChain.get<LowCut>(), lowCutCoefficients, chainSettings.LowCutSlope);
        UpdateCutFilter(monoChain.get<HighCut>(), highCutCoefficients, chainSettings.HighCutSlope);
        //single a repaint
        repaint();
    }
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    Component::paint(g);
    using namespace juce;
    // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colours::black);

    auto responseArea = getLocalBounds();

    auto W = responseArea.getWidth();
    auto H = responseArea.getHeight();

    //Get filter for chain
    auto& lowCut = monoChain.get<ChainPosition::LowCut>();
    auto& peak = monoChain.get<ChainPosition::Peak>();
    auto& highCut = monoChain.get<ChainPosition::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;
    mags.resize(W);


    for (int i = 0; i < W; ++i)
    {
        double mag = 1.0f;
        auto freq = mapToLog10(double(i) / double(W), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPosition::Peak>())
        {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowCut.isBypassed<0>())
        {
            mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowCut.isBypassed<1>())
        {
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowCut.isBypassed<2>())
        {
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowCut.isBypassed<3>())
        {
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if (!highCut.isBypassed<0>())
        {
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highCut.isBypassed<1>())
        {
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highCut.isBypassed<2>())
        {
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highCut.isBypassed<3>())
        {
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }


    Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin,outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.0f, 1.0f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
}


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
      highCutSlopeSliderAttachment(audioProcessor.apvts, "High Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : GetComps())
    {
        addAndMakeVisible(comp);
    }

    setSize(400, 300);
}

SampleEQAudioProcessorEditor::~SampleEQAudioProcessorEditor()
{
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
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33f);

    responseCurveComponent.setBounds(responseArea);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33f);
    // 66 -> 0.5 = 0.33
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5f);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);


    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    // 66 -> 0.5 = 0.33
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}
