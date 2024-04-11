/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TradeMarkEQAudioProcessor::TradeMarkEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

TradeMarkEQAudioProcessor::~TradeMarkEQAudioProcessor()
{
}

//==============================================================================
const juce::String TradeMarkEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TradeMarkEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TradeMarkEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TradeMarkEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TradeMarkEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TradeMarkEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TradeMarkEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TradeMarkEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TradeMarkEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void TradeMarkEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TradeMarkEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;

    spec.numChannels = 1;

    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);
}

void TradeMarkEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TradeMarkEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TradeMarkEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float>leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float>rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool TradeMarkEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TradeMarkEQAudioProcessor::createEditor()
{
    //return new TradeMarkEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TradeMarkEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TradeMarkEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout TradeMarkEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("LowCut Freq",
            "LowCut Freq",
            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
            20.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("HighCut Freq",
            "HighCut Freq",
            juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
            20000.f));

    //Low Peak
    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("LowPeak Freq",
            "LowPeak Freq",
            juce::NormalisableRange<float>(20.f, 500.f, 1.f, 1.f),
            250.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("LowPeak Gain",
            "LowPeak Gain",
            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
            0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("LowPeak Quality",
            "LowPeak Quality",
            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
            1.f));

    //Mid Low Peak
    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidLowPeak Freq",
            "MidLowPeak Freq",
            juce::NormalisableRange<float>(40.f, 1000.f, 1.f, 1.f),
            550.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidLowPeak Gain",
            "MidLowPeak Gain",
            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
            0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidLowPeak Quality",
            "MidLowPeak Quality",
            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
            1.f));

    //Mid Peak
    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidPeak Freq",
            "MidPeak Freq",
            juce::NormalisableRange<float>(125.f, 8000.f, 1.f, 1.f),
            1000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidPeak Gain",
            "MidPeak Gain",
            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
            0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidPeak Quality",
            "MidPeak Quality",
            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
            1.f));

    //Mid High Peak
    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidHighPeak Freq",
            "MidHighPeak Freq",
            juce::NormalisableRange<float>(200.f, 18000.f, 1.f, 1.f),
            12000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidHighPeak Gain",
            "MidHighPeak Gain",
            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
            0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("MidHighPeak Quality",
            "MidHighPeak Quality",
            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
            1.f));

    //High Peak
    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("HighPeak Freq",
            "HighPeak Freq",
            juce::NormalisableRange<float>(2000.f, 20000.f, 1.f, 1.f),
            17000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("HighPeak Gain",
            "HighPeak Gain",
            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
            0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>
        ("HighPeak Quality",
            "HighPeak Quality",
            juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
            1.f));

    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (6 + i * 6);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique < juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique < juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TradeMarkEQAudioProcessor();
}
