/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "pluginterfaces/vst/vsttypes.h"


#pragma region publicFunction

template <int Index, typename ChainType, typename CoefficientType>
void Update(ChainType& Chain, const CoefficientType& coefficients)
{
    UpdateCoefficients(Chain.template get<Index>().coefficients, coefficients[Index]);
    Chain.template setBypassed<Index>(false);
}

template <typename ChainType, typename CoefficientType>
void UpdateCutFilter(ChainType& Chain, const CoefficientType& coefficients,
                     const Slope& slope)
{
    //Close all Filter, single don't working
    Chain.template setBypassed<0>(true);
    Chain.template setBypassed<1>(true);
    Chain.template setBypassed<2>(true);
    Chain.template setBypassed<3>(true);

    switch (slope)
    {
    case Slope_12: Update<0>(Chain, coefficients);
        break;
    case Slope_24: Update<1>(Chain, coefficients);
        break;
    case Slope_36: Update<2>(Chain, coefficients);
        break;
    case Slope_48: Update<3>(Chain, coefficients);
        break;
    }
}


#pragma endregion


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

    // Low High Cut Butterworth Highpass
    UpdateFilters();

    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);


    // osc.initialise([](float x) { return std::sin(x); });
    // spec.numChannels = getTotalNumOutputChannels();
    // osc.prepare(spec);
    // osc.setFrequency(1000);
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

    // Low High Cut Butterworth Highpass

    UpdateFilters();

    juce::dsp::AudioBlock<float> block(buffer);

    // buffer.clear();
    // juce::dsp::ProcessContextReplacing<float> stereoContext(block);
    // osc.process(stereoContext);


    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

    //FFT Buffer
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);

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
    return new SampleEQAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SampleEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SampleEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        UpdateFilters();
    }
}
#pragma region Paramater
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
    settings.HighCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    settings.lowCutBypass = apvts.getRawParameterValue(lowCutBypass)->load() > 0.5f;
    settings.peakBypass = apvts.getRawParameterValue(peakByPass)->load() > 0.5f;
    settings.highCutBypass = apvts.getRawParameterValue(highCutBypass)->load() > 0.5f;
    // settings.lowCutBypass = apvts.getRawParameterValue(lowCutBypass)->load()>0.5f;

    return settings;
}

void UpdateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    // *ptr& 
    *old = *replacements;
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
        , 20000.0f));

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
        "HighCut Slope",
        "HighCut Slope",
        stringArray,
        0.0f
    ));


    layout.add(std::make_unique<juce::AudioParameterBool>(lowCutBypass,lowCutBypass,false));
    layout.add(std::make_unique<juce::AudioParameterBool>(peakByPass,peakByPass,false));
    layout.add(std::make_unique<juce::AudioParameterBool>(highCutBypass,highCutBypass,false));
    layout.add(std::make_unique<juce::AudioParameterBool>(analyzerByPass,analyzerByPass,false));

    return layout;
}

#pragma endregion

#pragma region Single Peak


void SampleEQAudioProcessor::UpdatePeakFilter(const ChainSettings& chainSettings)
{
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
    
    //Single Filter
    UpdateCoefficients(leftChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);
    UpdateCoefficients(rightChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);
}

void SampleEQAudioProcessor::UpdateHighCutFilters(const ChainSettings& chainSettings)
{
    auto HighCutCoefficients = makeHighCutFilters(chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<ChainPosition::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPosition::HighCut>();

    UpdateCutFilter(leftHighCut, HighCutCoefficients, chainSettings.HighCutSlope);
    UpdateCutFilter(rightHighCut, HighCutCoefficients, chainSettings.HighCutSlope);
}

void SampleEQAudioProcessor::UpdateLowCutFilters(const ChainSettings& chainSettings)
{
    auto LowCutCoefficients = makeLowCutFilters(chainSettings, getSampleRate());

    auto& leftLowCut = leftChain.get<ChainPosition::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPosition::LowCut>();
    
    UpdateCutFilter(leftLowCut, LowCutCoefficients, chainSettings.LowCutSlope);
    UpdateCutFilter(rightLowCut, LowCutCoefficients, chainSettings.LowCutSlope);
}


#pragma endregion

#pragma region Low High Cut IIR

void SampleEQAudioProcessor::UpdateFilters()
{
    const ChainSettings& chainSettings = getChainSettings(apvts);

    
    
    //High Cut   
    UpdateHighCutFilters(chainSettings);
    // Single Filter
    UpdatePeakFilter(chainSettings);
    // LowCut Butterworth Highpass
    UpdateLowCutFilters(chainSettings);


    // 设置旁通状态
    leftChain.setBypassed<ChainPosition::LowCut>(chainSettings.lowCutBypass);
    rightChain.setBypassed<ChainPosition::LowCut>(chainSettings.lowCutBypass);

    leftChain.setBypassed<ChainPosition::Peak>(chainSettings.peakBypass);
    rightChain.setBypassed<ChainPosition::Peak>(chainSettings.peakBypass);

    leftChain.setBypassed<ChainPosition::HighCut>(chainSettings.highCutBypass);
    rightChain.setBypassed<ChainPosition::HighCut>(chainSettings.highCutBypass);

    // DBG("LOW = " + juce::String(chainSettings.lowCutBypass ? "true" : "false"));
    // DBG("PEAK = " + juce::String(chainSettings.peakBypass ? "true" : "false"));
    // DBG("HIGH = " + juce::String(chainSettings.highCutBypass ? "true" : "false"));

}


#pragma endregion


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleEQAudioProcessor();
}
