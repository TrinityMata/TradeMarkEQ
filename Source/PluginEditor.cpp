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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
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
