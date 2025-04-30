/*
  ==============================================================================

    ResponseCurveComponent.h
    Created: 30 Apr 2025 11:36:15am
    Author:  tyzTang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RotarySliderWithLabels.h"

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
  void resized() override;
private:
  SampleEQAudioProcessor& audioProcessor;
  juce::Atomic<bool> parametersChanged{false};

  MonoChain monoChain;
  void UpdateChain();

  juce::Image background;
  juce::Rectangle<int> getRenderArea();
  juce::Rectangle<int> getAnalysisArea();

  // SingleChannelSampleFifo<SampleEQAudioProcessor::BlockType>* leftChannelFifo ;
  // SingleChannelSampleFifo<SampleEQAudioProcessor::BlockType>* rightChannelFifo ;
};