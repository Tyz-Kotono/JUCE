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
    UpdateChain();
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
        UpdateChain();
        repaint();
    }
}

void ResponseCurveComponent::UpdateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    UpdateCoefficients(monoChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilters(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilters(chainSettings, audioProcessor.getSampleRate());

    UpdateCutFilter(monoChain.get<LowCut>(), lowCutCoefficients, chainSettings.LowCutSlope);
    UpdateCutFilter(monoChain.get<HighCut>(), highCutCoefficients, chainSettings.HighCutSlope);
    //single a repaint
}


void ResponseCurveComponent::paint(juce::Graphics& g)
{
    Component::paint(g);
    using namespace juce;
    // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colours::black);
    g.drawImage(background, getLocalBounds().toFloat());


    // auto responseArea = getLocalBounds();
    //limit line
    auto responseArea = getAnalisisArea();

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


    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 2.0f, 1.0f);

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
}

void ResponseCurveComponent::resized()
{
    Component::resized();
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);

    Array<float> freqs{20, /*30, 40,*/ 50, 100, 200, /*300, 400,*/ 500, 1000, 2000, /*4000, */5000, 10000, 20000};

    //Draw Line

    auto renderArea = getAnalisisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.0f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::dimgrey);
    for (auto f : xs)
    {
        g.drawVerticalLine(f, top, bottom);
    }

    Array<float> gains{-24, -12, 0, 12, 24};

    for (auto gDb : gains)
    {
        auto y = jmap(gDb, -24.0f, 24.0f, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }

    g.drawRect(getAnalisisArea());


    //Font
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool abbK = false;
        String str;
        if (f > 999.f)
        {
            abbK = true;
            f /= 1000.0f;
        }
        str << f;
        if (abbK)
            str << "k";
        str << "Hz";


        // box
        auto textWidth = g.getCurrentFont().getStringWidth(str);

        juce::Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
}


juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    // bounds.reduce(JUCE_LIVE_CONSTANT(5),JUCE_LIVE_CONSTANT(13));
    // bounds.reduce(12, 10);
    bounds.removeFromTop(12);
    bounds.removeFromBottom(4);
    bounds.removeFromLeft(12);
    bounds.removeFromRight(12);
    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalisisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
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

    setSize(600, 400);
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

    float hRatio = 0.33f; // JUCE_LIVE_CONSTANT(33)/100.0f;

    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(5);
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
