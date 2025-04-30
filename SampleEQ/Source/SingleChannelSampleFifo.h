/*
  ==============================================================================

    SingleChannelSampleFifo.h
    Created: 30 Apr 2025 10:36:51am
    Author:  tyzTang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


enum Channel
{
    Right,
    Left
};

#include <array>

template <typename T>
struct Fifo
{
    void preapre(int numChannels, int numSample)
    {
        for (auto& buffer : buffers)
        {
            buffer.setSize(
                numChannels,
                numSample,
                false,
                true,
                true
            );
            buffer.clear();
        }
    }

    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if (write.blockSize1 > 0)
        {
            buffers[write.startIndex1] = t;
            return true;
        }
        return false;
    }

    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if (read.blockSize1 > 0)
        {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }
    int getNumAvailableForReading() const
    {
        return fifo.getNumReady();
    }
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo{Capacity};
};

//  using BlockType = juce::AudioBuffer<float>;
template <typename BlockType>
struct SingleChannelSampleFifo  // : juce::AudioBuffer<float>
{
    juce::AudioBuffer<float> buffer;
    SingleChannelSampleFifo(Channel ch) // : juce::channelToUse(ch)
    {
        prepared.set(false);
    }

    void Update(const BlockType& buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        auto* channelPtr = buffer.getReadPointer(channelToUse);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    //Set the Sample Block by prepareToPlay
    void Prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);

        bufferToFill.setSize
        (
            1,
            bufferSize,
            false,
            true,
            true
        );
        audioBufferFifo.preapre(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }

    int GetNumCompleteBufferAvailable() const
    {
        return audioBufferFifo.getNumAvailableForReading();
    }

    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }

    //
    bool GetAudioBuffer(BlockType& buffer)
    {
        return audioBufferFifo.pull(buffer);
    }

private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;

    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;

    void pushNextSampleIntoFifo(float Sample)
    {
        if (fifoIndex == bufferToFill.getNumSamples())
        {
            auto ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }
        bufferToFill.setSample(0, fifoIndex, Sample);
        ++fifoIndex;
    }
};
