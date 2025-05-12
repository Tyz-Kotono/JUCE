/*
  ==============================================================================

    ResponseCurveComponent.cpp
    Created: 30 Apr 2025 11:36:15am
    Author:  tyzTang

  ==============================================================================
*/

#include "ResponseCurveComponent.h"

ResponseCurveComponent::ResponseCurveComponent(SampleEQAudioProcessor& p) :
    audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)

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
    if(shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        updateFFT(fftBounds, sampleRate);
    }
 

    //Updata
    if (parametersChanged.compareAndSetBool(false, true))
    {
        // Update momo chain
        UpdateChain();
        // signal a repaint 
        // repaint();
    }

    repaint();
}


void ResponseCurveComponent::UpdateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());

    monoChain.setBypassed<ChainPosition::LowCut>(chainSettings.lowCutBypass);
    monoChain.setBypassed<ChainPosition::Peak>(chainSettings.peakBypass);
    monoChain.setBypassed<ChainPosition::HighCut>(chainSettings.highCutBypass);

    UpdateCoefficients(monoChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilters(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilters(chainSettings, audioProcessor.getSampleRate());

    UpdateCutFilter(monoChain.get<LowCut>(), lowCutCoefficients, chainSettings.LowCutSlope);
    UpdateCutFilter(monoChain.get<HighCut>(), highCutCoefficients, chainSettings.HighCutSlope);
    //single a repaint
}

void ResponseCurveComponent::updateFFT(juce::Rectangle<float> fftBounds, double sampleRate)
{
    leftPathProducer.process(fftBounds, sampleRate);
    rightPathProducer.process(fftBounds, sampleRate);
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    Component::paint(g);
    using namespace juce;

    paintResponseCurve(g);

    if(shouldShowFFTAnalysis)
    {
        //Limit the path in the area 
        auto leftChannelFFTPath = leftPathProducer.getPath();
        auto rightChannelFFTPath = rightPathProducer.getPath();
        leftChannelFFTPath.
            applyTransform(AffineTransform().translation(getAnalysisArea().getX(), getAnalysisArea().getY()));
        rightChannelFFTPath.
            applyTransform(AffineTransform().translation(getAnalysisArea().getX(), getAnalysisArea().getY()));


        g.setColour(Colours::blue);
        g.strokePath(leftPathProducer.getPath(), PathStrokeType(1));
        g.setColour(Colours::red);
        g.strokePath(rightPathProducer.getPath(), PathStrokeType(1));
    }
    
    
    // g.setColour(Colours::red);
    // g.drawRect(getLocalBounds());

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 2.0f, 1.0f);
}

void ResponseCurveComponent::paintResponseCurve(juce::Graphics& g)
{
    using namespace juce;
    // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colours::black);
    g.drawImage(background, getLocalBounds().toFloat());


    // auto responseArea = getLocalBounds();
    //limit line
    auto responseArea = getAnalysisArea();

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

        if (!monoChain.isBypassed<ChainPosition::LowCut>())
        {
             
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
        }
       
        if (!monoChain.isBypassed<ChainPosition::HighCut>())
        {
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

    auto renderArea = getAnalysisArea();
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

    g.drawRect(getAnalysisArea());


    //Font
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    //frequency
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


        //gain
        for (auto gDb : gains)
        {
            auto y = jmap(gDb, -24.0f, 24.0f, float(bottom), float(top));

            String str;
            if (gDb > 0)
                str << "+";
            str << gDb;

            //font right
            auto textWidth = g.getCurrentFont().getStringWidth(str);

            juce::Rectangle<int> r;

            float widthScale = 1.25f; // 宽度为文本的1.5倍
            r.setSize(textWidth * widthScale, fontHeight);
            r.setX(getWidth() - r.getWidth());
            r.setCentre(r.getCentre().x, y);

            g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
            g.drawFittedText(str, r, juce::Justification::centred, 1);

            //Font left
            str.clear();
            str << (gDb - 24.0f);
            r.setX(1);
            textWidth = g.getCurrentFont().getStringWidth(str);
            r.setSize(textWidth, fontHeight);
            g.drawFittedText(str, r, juce::Justification::centred, 1);
        }
    }
}



juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    // bounds.reduce(JUCE_LIVE_CONSTANT(5),JUCE_LIVE_CONSTANT(13));
    // bounds.reduce(12, 10);
    bounds.removeFromTop(12);
    bounds.removeFromBottom(4);
    bounds.removeFromLeft(24);
    bounds.removeFromRight(24);
    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}
