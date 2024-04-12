/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TradeMarkEQAudioProcessorEditor::TradeMarkEQAudioProcessorEditor (TradeMarkEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    lowPeakFreqSliderAttachment(audioProcessor.apvts, "LowPeak Freq", lowPeakFreqSlider),
    lowPeakGainSliderAttachment(audioProcessor.apvts, "LowPeak Gain", lowPeakGainSlider),
    lowPeakQualitySliderAttachment(audioProcessor.apvts, "LowPeak Quality", lowPeakQualitySlider),
    midlowPeakFreqSliderAttachment(audioProcessor.apvts, "MidLowPeak Freq", midlowPeakFreqSlider),
    midlowPeakGainSliderAttachment(audioProcessor.apvts, "MidLowPeak Gain", midlowPeakGainSlider),
    midlowPeakQualitySliderAttachment(audioProcessor.apvts, "MidLowPeak Quality", midlowPeakQualitySlider),
    midPeakFreqSliderAttachment(audioProcessor.apvts, "MidPeak Freq", midPeakFreqSlider),
    midPeakGainSliderAttachment(audioProcessor.apvts, "MidPeak Gain", midPeakGainSlider),
    midPeakQualitySliderAttachment(audioProcessor.apvts, "MidPeak Quality", midPeakQualitySlider),
    midhighPeakFreqSliderAttachment(audioProcessor.apvts, "MidHighPeak Freq", midhighPeakFreqSlider),
    midhighPeakGainSliderAttachment(audioProcessor.apvts, "MidHighPeak Gain", midhighPeakGainSlider),
    midhighPeakQualitySliderAttachment(audioProcessor.apvts, "MidHighPeak Quality", midhighPeakQualitySlider),
    highPeakFreqSliderAttachment(audioProcessor.apvts, "HighPeak Freq", highPeakFreqSlider),
    highPeakGainSliderAttachment(audioProcessor.apvts, "HighPeak Gain", highPeakGainSlider),
    highPeakQualitySliderAttachment(audioProcessor.apvts, "HighPeak Quality", highPeakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    setSize(600, 400);
}

TradeMarkEQAudioProcessorEditor::~TradeMarkEQAudioProcessorEditor()
{
}

//==============================================================================
void TradeMarkEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    auto bounds = getLocalBounds();

    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& lowpeak = monoChain.get<ChainPositions::LowPeak>();
    auto& midlowpeak = monoChain.get<ChainPositions::MidLowPeak>();
    auto& midpeak = monoChain.get<ChainPositions::MidPeak>();
    auto& midhighpeak = monoChain.get<ChainPositions::MidHighPeak>();
    auto& highpeak = monoChain.get<ChainPositions::HighPeak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> mags;

    mags.resize(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 2000.0);

        if (!monoChain.isBypassed<ChainPositions::LowPeak>())
            mag *= lowpeak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::MidLowPeak>())
            mag *= midlowpeak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::MidPeak>())
            mag *= midpeak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::MidHighPeak>())
            mag *= midhighpeak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!monoChain.isBypassed<ChainPositions::HighPeak>())
            mag *= highpeak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responsiveCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
        {
            return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responsiveCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responsiveCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(responsiveCurve, PathStrokeType(2.f));

}

void TradeMarkEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);

    lowPeakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    lowPeakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    lowPeakQualitySlider.setBounds(bounds);

}

void TradeMarkEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void TradeMarkEQAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //update the monochain
        //signal a repaint
    }
}

std::vector<juce::Component*> TradeMarkEQAudioProcessorEditor::getComps()
{
    return
    {
        &lowPeakFreqSlider,
        &lowPeakGainSlider,
        &lowPeakQualitySlider,

        &midlowPeakFreqSlider,
        &midlowPeakGainSlider,
        &midlowPeakQualitySlider,

        &midPeakFreqSlider,
        &midPeakGainSlider,
        &midPeakQualitySlider,

        &midhighPeakFreqSlider,
        &midhighPeakGainSlider,
        &midhighPeakQualitySlider,

        &highPeakFreqSlider,
        &highPeakGainSlider,
        &highPeakQualitySlider,

        &lowCutFreqSlider,
        & highCutFreqSlider,
        & lowCutSlopeSlider,
        & highCutSlopeSlider
    };
}
