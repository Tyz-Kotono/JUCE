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

    auto center = bounds.getCentre();

    Path p;
    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    //check
    jassert(rotaryStartAngle < rotaryEndAngle);
    auto sliderAngRad = jmap(sliderPosProportional,
                             0.f, 1.f,
                             rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad,center.getX(),center.getY()));
    graphics.fillPath(p);
    
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    auto startAng = radiansToDegrees(180.0f + 45.0f);
    auto endAng = radiansToDegrees(180.0f - 45.0f) + MathConstants<float>::twoPi;

    auto range = getRange();
    auto sliderBounds = getSliderBounds();


    getLookAndFeel().
        drawRotarySlider(g,
                         sliderBounds.getX(), sliderBounds.getY(),
                         sliderBounds.getWidth(), sliderBounds.getHeight(),
                         jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                         startAng, endAng,
                         *this
        );
}


void RotarySliderWithLabels::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    return getLocalBounds();
}
