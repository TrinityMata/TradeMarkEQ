/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(TradeMarkEQAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { };

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    TradeMarkEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    MonoChain monoChain;

};

//==============================================================================
/**
*/
class TradeMarkEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TradeMarkEQAudioProcessorEditor (TradeMarkEQAudioProcessor&);
    ~TradeMarkEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TradeMarkEQAudioProcessor& audioProcessor;

    CustomRotarySlider 
        lowPeakFreqSlider,
        lowPeakGainSlider,
        lowPeakQualitySlider,

        midlowPeakFreqSlider,
        midlowPeakGainSlider,
        midlowPeakQualitySlider,

        midPeakFreqSlider,
        midPeakGainSlider,
        midPeakQualitySlider,

        midhighPeakFreqSlider,
        midhighPeakGainSlider,
        midhighPeakQualitySlider,

        highPeakFreqSlider,
        highPeakGainSlider,
        highPeakQualitySlider,

        lowCutFreqSlider,
        highCutFreqSlider,
        lowCutSlopeSlider,
        highCutSlopeSlider;

    ResponseCurveComponent responseCurveComponent;

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment 
        lowPeakFreqSliderAttachment,
        lowPeakGainSliderAttachment,
        lowPeakQualitySliderAttachment,

        midlowPeakFreqSliderAttachment,
        midlowPeakGainSliderAttachment,
        midlowPeakQualitySliderAttachment,

        midPeakFreqSliderAttachment,
        midPeakGainSliderAttachment,
        midPeakQualitySliderAttachment,

        midhighPeakFreqSliderAttachment,
        midhighPeakGainSliderAttachment,
        midhighPeakQualitySliderAttachment,

        highPeakFreqSliderAttachment,
        highPeakGainSliderAttachment,
        highPeakQualitySliderAttachment,

        lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        lowCutSlopeSliderAttachment,
        highCutSlopeSliderAttachment;

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TradeMarkEQAudioProcessorEditor)
};
