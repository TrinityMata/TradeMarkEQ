/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_6,
    Slope_12,
    Slope_18,
    Slope_24
};

struct ChainSettings
{
    float lowPeakFreq{ 0 }, lowPeakGainInDecibels{ 0 }, lowPeakQuality{ 1.f };
    float midlowPeakFreq{ 0 }, midlowPeakGainInDecibels{ 0 }, midlowPeakQuality{ 1.f };
    float midPeakFreq{ 0 }, midPeakGainInDecibels{ 0 }, midPeakQuality{ 1.f };
    float midhighPeakFreq{ 0 }, midhighPeakGainInDecibels{ 0 }, midhighPeakQuality{ 1.f };
    float highPeakFreq{ 0 }, highPeakGainInDecibels{ 0 }, highPeakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{ Slope::Slope_6 }, highCutSlope{ Slope::Slope_6 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);


//==============================================================================
/**
*/
class TradeMarkEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TradeMarkEQAudioProcessor();
    ~TradeMarkEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr,"Parameters", createParameterLayout() };

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, Filter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        LowCut,
        LowPeak,
        MidLowPeak,
        MidPeak,
        MidHighPeak,
        HighPeak,
        HighCut
    };

    void updatePeakFilters(const ChainSettings& chainSettings);

    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }

    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& chain,
        const CoefficientType& coefficients,
        const Slope& slope)
    {
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        chain.template setBypassed<2>(true);
        chain.template setBypassed<3>(true);

        switch (slope)
        {
            case Slope_24:
            {
                update<3>(chain, coefficients);
            }
            case Slope_18:
            {
                update<2>(chain, coefficients);
            }
            case Slope_12:
            {
                update<1>(chain, coefficients);
            }
            case Slope_6:
            {
                update<0>(chain, coefficients);
            }
        }
    }

    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);

    void updateFilters();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TradeMarkEQAudioProcessor)
};
