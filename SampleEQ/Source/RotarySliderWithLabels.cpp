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

void LookAndFeel::drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height,
                                   float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                                   juce::Slider& slider)
{
    LookAndFeel_V4::drawRotarySlider(graphics, x, y, width, height, sliderPosProportional, rotaryStartAngle,
                                     rotaryEndAngle, slider);
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    //Draw Ellipse and Edge
    graphics.setColour(Colour(97u, 18u, 167u));
    graphics.fillEllipse(bounds);

    graphics.setColour(Colour(255u, 154u, 1u));
    graphics.drawEllipse(bounds, 1.0f);


    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();

        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 2.0f);


        //check
        jassert(rotaryStartAngle < rotaryEndAngle);
        //Map to the Angle range (radian system) based on the slider ratio value
        auto sliderAngRad = jmap(sliderPosProportional,
                                 0.f, 1.f,
                                 rotaryStartAngle, rotaryEndAngle);

        //Angle Pivot
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        graphics.fillPath(p);


        //font
        graphics.setFont(rswl->getTextHeight());
        auto text = rswl->GetDisplayString();
        auto strWidth = graphics.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        graphics.setColour(Colours::black);
        graphics.fillRect(r);

        graphics.setColour(Colours::white);
        graphics.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    auto startAng = MathConstants<float>::pi + MathConstants<float>::pi * 0.25f;
    auto endAng = startAng + MathConstants<float>::pi * 1.5f;
    

    auto range = getRange();
    auto sliderBounds = getSliderBounds();

    //Debug Line of Edge
    // g.setColour(Colours::red);
    // g.drawRect(getLocalBounds());
    // g.setColour(Colours::yellow);
    // g.drawRect(sliderBounds);

    getLookAndFeel().
        drawRotarySlider(g,
                         sliderBounds.getX(), sliderBounds.getY(),
                         sliderBounds.getWidth(), sliderBounds.getHeight(),
                         jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                         startAng, endAng,
                         *this
        );


    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i <= numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <=1.0f);

        auto ang = jmap(pos,
                        0.0f, 1.0f,
                        startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

      
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}


void RotarySliderWithLabels::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    // return getLocalBounds();
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> rect;
    rect.setSize(size, size);

    rect.setCentre(bounds.getCentreX(), 0);
    rect.setY(2);
    return rect;
}

juce::String RotarySliderWithLabels::GetDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if (val > 999.f)
        {
            val /= 1000.0f;
            addK = true;
        }

        str = juce::String(val, (addK) ? 2 : 0);
    }
    else
    {
        jassertfalse;
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "K";

        str << suffix;
    }

    return str;
}


