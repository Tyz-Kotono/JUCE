/*
  ==============================================================================

    PathProducer.cpp
    Created: 30 Apr 2025 4:51:53pm
    Author:  tyzTang

  ==============================================================================
*/

#include "PathProducer.h"

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;


    while (ChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (ChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();
            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size
            );
            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size
            );

            ChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.0f);
        }
    }

    /*
  if there are FFT data buffers to pull
  if we can pull a buffer
  generate apth
   */

    // const auto fftBounds =bounds.toFloat();
    const auto fftSize = ChannelFFTDataGenerator.getFFTSize();


    /*
     48000/4048 = =23hz this is the bin width
     */

    const auto binWidth = sampleRate / (double)fftSize;

    //have data block
    while (ChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        //can get the fft data
        if (ChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.0f);
        }
    }

    // leftChannelFFTPath.

    /*
    while there are paths that can be pll
    pull as many as we can
    display the most recent path
    */

    while (pathProducer.getNumPathsAvailable())
    {
        pathProducer.getPath(ChannelFFTPath);
    }

    
}
