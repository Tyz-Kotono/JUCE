/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "SingleChannelSampleFifo.h"

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
    bool lowCutBypass{false},peakBypass{false},highCutBypass{false};
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

    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo{Channel::Left};
    SingleChannelSampleFifo<BlockType> rightChannelFifo{Channel::Right};
private:
    //==============================================================================


    MonoChain leftChain, rightChain;

    void UpdateFilters();

    //Single Filter
    void UpdatePeakFilter(const ChainSettings& chainSettings);

    void UpdateHighCutFilters(const ChainSettings& chainSettings);
    void UpdateLowCutFilters(const ChainSettings& chainSettings);

    juce::dsp::Oscillator<float> osc;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEQAudioProcessor)
};
