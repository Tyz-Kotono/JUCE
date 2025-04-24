/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "pluginterfaces/vst/vsttypes.h"

//==============================================================================
SampleEQAudioProcessor::SampleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

SampleEQAudioProcessor::~SampleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SampleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SampleEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SampleEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SampleEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SampleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SampleEQAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SampleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SampleEQAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SampleEQAudioProcessor::getProgramName(int index)
{
    return {};
}

void SampleEQAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SampleEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSetting = getChainSettings(apvts);

    auto peakCoefficients =
        juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            chainSetting.peakFreq,
            chainSetting.peakQuality,
            juce::Decibels::decibelsToGain(chainSetting.peakGainInDecibels)
        );


    //Single Filter
    *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;


    //
    auto CutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod
    (
        chainSetting.lowCutFreq,
        sampleRate,
        2 * (chainSetting.LowCutSlope + 1)
    );

    auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    IIRHighpassCutFilter(leftLowCut, chainSetting, CutCoefficients);
    IIRHighpassCutFilter(rightLowCut, chainSetting, CutCoefficients);
    // leftLowCut.setBypassed<0>(true);
    // leftLowCut.setBypassed<1>(true);
    // leftLowCut.setBypassed<2>(true);
    // leftLowCut.setBypassed<3>(true);
    //
    // switch (chainSetting.LowCutSlope)
    // {
    // case Slope_12:
    //     *leftLowCut.get<0>().coefficients = *CutCoefficients[0];
    //     leftLowCut.setBypassed<0>(false);
    //     break;
    // case Slope_24:
    //     *leftLowCut.get<0>().coefficients = *CutCoefficients[0];
    //     leftLowCut.setBypassed<0>(false);
    //     *leftLowCut.get<1>().coefficients = *CutCoefficients[1];
    //     leftLowCut.setBypassed<1>(false);
    //     break;
    // case Slope_36:
    //     *leftLowCut.get<0>().coefficients = *CutCoefficients[0];
    //     leftLowCut.setBypassed<0>(false);
    //     *leftLowCut.get<1>().coefficients = *CutCoefficients[1];
    //     leftLowCut.setBypassed<1>(false);
    //     *leftLowCut.get<2>().coefficients = *CutCoefficients[2];
    //     leftLowCut.setBypassed<2>(false);
    //
    //     break;
    // case Slope_48:
    //     *leftLowCut.get<0>().coefficients = *CutCoefficients[0];
    //     leftLowCut.setBypassed<0>(false);
    //     *leftLowCut.get<1>().coefficients = *CutCoefficients[1];
    //     leftLowCut.setBypassed<1>(false);
    //     *leftLowCut.get<2>().coefficients = *CutCoefficients[2];
    //     leftLowCut.setBypassed<2>(false);
    //     *leftLowCut.get<3>().coefficients = *CutCoefficients[3];
    //     leftLowCut.setBypassed<3>(false);
    //     break;
    // }
}

void SampleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SampleEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SampleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto chainSetting = getChainSettings(apvts);

    auto peakCoefficients =
        juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            getSampleRate(),
            chainSetting.peakFreq,
            chainSetting.peakQuality,
            juce::Decibels::decibelsToGain(chainSetting.peakGainInDecibels)
        );

    //Single Filter
    *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;


    auto CutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod
    (
        chainSetting.lowCutFreq,
        getSampleRate(),
        2 * (chainSetting.LowCutSlope + 1)
    );

    auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    IIRHighpassCutFilter(leftLowCut, chainSetting, CutCoefficients);
    IIRHighpassCutFilter(rightLowCut, chainSetting, CutCoefficients);


    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);


    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool SampleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SampleEQAudioProcessor::createEditor()
{
    // return new SampleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SampleEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SampleEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    //Normalize the Value Range
    // apvts.getParameter("LowCut Freq")->getValue();

    //return the value Range 
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.LowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("High Slope")->load());


    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout SampleEQAudioProcessor::CreateParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Freq", "LowCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.0f, 0.25f)
        , 20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCut Freq", "HighCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.0f, 0.25f)
        , 20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Freq", "Peak Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.0f, 0.25f)
        , 750.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Gain", "Peak Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f)
        , 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Quality", "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f)
        , 1.0f));


    //12 equal temperament
    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i * 12);
        str << "db/out";
        stringArray.add(str);
    }


    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "LowCut Slope",
        "LowCut Slope",
        stringArray,
        0.0f
    ));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "High Slope",
        "High Slope",
        stringArray,
        0.0f
    ));


    return layout;
}

void SampleEQAudioProcessor::IIRHighpassCutFilter(CutFilter& CutFilter, ChainSettings Setting,
                                                  juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>>
                                                  CutCoefficients)
{
    //Close all Filter, single don't working
    CutFilter.setBypassed<0>(true);
    CutFilter.setBypassed<1>(true);
    CutFilter.setBypassed<2>(true);
    CutFilter.setBypassed<3>(true);

    switch (Setting.LowCutSlope)
    {
    case Slope_12:
        *CutFilter.get<0>().coefficients = *CutCoefficients[0];
        CutFilter.setBypassed<0>(false);
        break;
    case Slope_24:
        *CutFilter.get<0>().coefficients = *CutCoefficients[0];
        CutFilter.setBypassed<0>(false);
        *CutFilter.get<1>().coefficients = *CutCoefficients[1];
        CutFilter.setBypassed<1>(false);
        break;
    case Slope_36:
        *CutFilter.get<0>().coefficients = *CutCoefficients[0];
        CutFilter.setBypassed<0>(false);
        *CutFilter.get<1>().coefficients = *CutCoefficients[1];
        CutFilter.setBypassed<1>(false);
        *CutFilter.get<2>().coefficients = *CutCoefficients[2];
        CutFilter.setBypassed<2>(false);

        break;
    case Slope_48:
        *CutFilter.get<0>().coefficients = *CutCoefficients[0];
        CutFilter.setBypassed<0>(false);
        *CutFilter.get<1>().coefficients = *CutCoefficients[1];
        CutFilter.setBypassed<1>(false);
        *CutFilter.get<2>().coefficients = *CutCoefficients[2];
        CutFilter.setBypassed<2>(false);
        *CutFilter.get<3>().coefficients = *CutCoefficients[3];
        CutFilter.setBypassed<3>(false);
        break;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleEQAudioProcessor();
}
