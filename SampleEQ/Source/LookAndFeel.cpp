/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 12 May 2025 7:07:33pm
    Author:  tyzTang

  ==============================================================================
*/

#include "LookAndFeel.h"

#include "PowerButton.h"
#include "RotarySliderWithLabels.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& graphics, int x, int y, int width, int height,
                                   float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                                   juce::Slider& slider)
{
    LookAndFeel_V4::drawRotarySlider(graphics, x, y, width, height, sliderPosProportional, rotaryStartAngle,
                                     rotaryEndAngle, slider);
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    auto enabled = slider.isEnabled();

    //Draw Ellipse and Edge
    graphics.setColour(enabled ? Colour(97u, 18u, 167u) : Colours::darkgrey);
    graphics.fillEllipse(bounds);

    graphics.setColour(enabled ? Colour(255u, 154u, 1u) : Colours::darkgrey);
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

void LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& togglebutton,
                                   bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    //remove the default bool box
    // LookAndFeel_V4::drawToggleButton(g, togglebutton, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);


    if (auto* pd = dynamic_cast<PowerButton*>(&togglebutton))
    {
        DrawPowerButton(g, togglebutton);
    }
    else if (auto* pd = dynamic_cast<AnalyzerButton*>(&togglebutton))
    {
        DrawAnalyzerButton(g, *pd);
    }
}

void LookAndFeel::DrawPowerButton(juce::Graphics& g, juce::ToggleButton& togglebutton)
{
    using namespace juce;

    Path powerButton;
    auto bounds = togglebutton.getLocalBounds();

    //Debug Edge
    // g.setColour(Colours::red);
    // g.drawRect(bounds);

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) - 4;
    auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

    float ang = 30.0f;
    size -= 6;

    powerButton.addCentredArc(r.getCentreX(),
                              r.getCentreY(),
                              size * 0.5,
                              size * 0.5,
                              0.0f,
                              degreesToRadians(ang),
                              degreesToRadians(360.0f - ang),
                              true
    );

    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre());

    PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
    // DBG("LOW = " + juce::String(togglebutton.getState() ? "true" : "false"));

    g.setColour(togglebutton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u));
    g.strokePath(powerButton, pst);

    g.drawEllipse(r, 2.0f);
}

void LookAndFeel::DrawAnalyzerButton(juce::Graphics& g, AnalyzerButton& togglebutton)
{
    using namespace juce;

    g.setColour(togglebutton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u));


    auto bounds = togglebutton.getLocalBounds();
    auto insetRect = bounds.reduced(4);

    g.strokePath(togglebutton.randomPath, PathStrokeType(1.0f));
}
