/*
  ==============================================================================

    PathProducer.h
    Created: 30 Apr 2025 4:51:53pm
    Author:  tyzTang

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"
#include "SingleChannelSampleFifo.h"

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<SampleEQAudioProcessor::BlockType>& scsf): ChannelFifo(&scsf)
    {
        
        /*
         48000/4048 = =23hz
         */

        ChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, ChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return ChannelFFTPath; }

private:
    juce::AudioBuffer<float> monoBuffer;

    SingleChannelSampleFifo<SampleEQAudioProcessor::BlockType>* ChannelFifo;
    FFTDataGenerator<std::vector<float>> ChannelFFTDataGenerator;
    AnalyzerPathGenerator<juce::Path> pathProducer;

    juce::Path ChannelFFTPath;
    SingleChannelSampleFifo<SampleEQAudioProcessor::BlockType>* rightChannelFifo;
};
