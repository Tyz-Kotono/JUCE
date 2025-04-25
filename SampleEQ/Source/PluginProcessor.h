/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

    Slope  LowCutSlope{Slope::Slope_12}, highCutSlope{Slope::Slope_12};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

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

    //Butterworth Highpass
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;


  

    enum ChainPosition
    {
        LowCut,
        Peak,
        HighCut
    };


    //Single Filter
    void UpdatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void UpdateCoefficients(Coefficients& old, const Coefficients& replacements);

    //IIR
    void UpdateIIRHighpassHigh(const ChainSettings& chainSettings);
    void IIRHighpassCutFilter(CutFilter& CutFilter, ChainSettings Setting,
                              juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> CutCoefficients);

    template<typename  ChainType,typename CoefficientType>
    void UpdateCutFilter(ChainType& leftLowCut,
                        const CoefficientType& cutCoefficients,
                        // const ChainSettings& chainSettings,
                        const Slope& lowCutSlope
    );
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleEQAudioProcessor)
};

