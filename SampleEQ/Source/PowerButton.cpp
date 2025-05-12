/*
  ==============================================================================

    PowerButton.cpp
    Created: 12 May 2025 7:01:04pm
    Author:  tyzTang

  ==============================================================================
*/

#include "PowerButton.h"

void AnalyzerButton::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds();

    auto insetRect = bounds.reduced(4);

    Random r;

    randomPath.startNewSubPath(insetRect.getX(),
                               insetRect.getY() + insetRect.getHeight() * r.nextFloat()
    );

    for (auto x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2)
    {
        randomPath.lineTo(x,
                          insetRect.getY() + insetRect.getHeight() *
                          r.nextFloat()
        );
    }
}
