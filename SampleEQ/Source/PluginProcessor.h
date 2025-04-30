/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

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

private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo{Capacity};
};

template <typename BlockType>
struct SingleChannelSampleFifo
{
    SingleChannelSampleFifo(Channel ch) : juce::channelToUse(ch)
    {
        prepared.set(false);
    }

    void Update(const BlockType& buffer)
    {
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse);
        auto* channelPtr = buffer.getReadPointer(channelToUse);

        for (int i = 0; i < buffer.getNmSamples; ++i)
        {
            pushNextSampleIntoFifo(channelPtr[i]);
        }
    }

    void Prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);

        bufferToFill.setSzie
        (
            1,
            bufferSize,
            false,
            true,
            true
        );
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }

    int GetNumCompleteBufferAvailable() const
    {
        return audioBufferFifo.getNumAvailableForReading;
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
        if (fifoIndex == buffertoFill.getNumSample())
        {
            auto ok = audioBufferFifo.push(buffertoFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
        }
        buffertoFill.setSample(0, fifoIndex, Sample);
        ++fifoIndex;
    }
};


enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48,
};

struct ChainSettings
{
    float peakFreq{0}, peakGainInDecibels{0}, peakQuality{1.0f};
    float lowCutFreq{0}, highCutFreq{0};

    Slope LowCutSlope{Slope::Slope_12}, HighCutSlope{Slope::Slope_12};
};


//Butterworth Highpass
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

enum ChainPosition
{
    LowCut,
    Peak,
    HighCut
};

using Coefficients = Filter::CoefficientsPtr;

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//Single Frequency 
void UpdateCoefficients(Coefficients& old, const Coefficients& replacements);

//Low High Cut
template <int Index, typename ChainType, typename CoefficientType>
void Update(ChainType& Chain, const CoefficientType& coefficients);

template <typename ChainType, typename CoefficientType>
void UpdateCutFilter(ChainType& leftLowCut, const CoefficientType& cutCoefficients, const Slope& lowCutSlope);


//Filter

#pragma region Filter for Frequency

inline Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return
        juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            chainSettings.peakFreq,
            chainSettings.peakQuality,
            juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels)
        );
}

inline auto makeLowCutFilters(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod
    (
        chainSettings.lowCutFreq,
        sampleRate,
        2 * (chainSettings.LowCutSlope + 1)
    );
}

inline auto makeHighCutFilters(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod
    (
        chainSettings.highCutFreq,
        sampleRate,
        2 * (chainSettings.HighCutSlope + 1)
    );
}

#pragma endregion


//==============================================================================
/**
*/
class SampleEQAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SampleEQAudioProcessor();
    ~SampleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //Kotono
    //https://docs.juce.com/master/classAudioProcessorParameter.html
    static juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout();
    juce::AudioProcessorValueTreeState apvts
    {
        *this,
        nullptr,
        "Parameter",
        CreateParameterLayout(),
    };

private:
    //==============================================================================


    MonoChain leftChain, rightChain;

    void UpdateFilters();

    //Single Filter
    void UpdatePeakFilter(const ChainSettings& chainSettings);

    void UpdateHighCutFilters(const ChainSettings& chainSettings);
    void UpdateLowCutFilters(const ChainSettings& chainSettings);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEQAudioProcessor)
};
