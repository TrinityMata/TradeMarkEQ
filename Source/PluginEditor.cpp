/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEngAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(97u, 18u, 167u));
    g.fillEllipse(bounds);

    g.setColour(Colour(255u, 154u, 1u));
    g.drawEllipse(bounds, 1.f);

    auto center = bounds.getCentre();

    Path p;

    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    jassert(rotaryStartAngle < rotaryEngAngle);

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEngAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

    g.fillPath(p);
}

//===============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    // Grid lines
    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(TradeMarkEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}
void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        DBG("params changed");
        //update the monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto lowPeakCoefficients = makeLowPeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::LowPeak>().coefficients, lowPeakCoefficients);
        auto midlowPeakCoefficients = makeMidLowPeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::MidLowPeak>().coefficients, midlowPeakCoefficients);
        auto midPeakCoefficients = makeMidPeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::MidPeak>().coefficients, midPeakCoefficients);
        auto midhighPeakCoefficients = makeMidHighPeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::MidHighPeak>().coefficients, midhighPeakCoefficients);
        auto highPeakCoefficients = makeHighPeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::HighPeak>().coefficients, highPeakCoefficients);

        auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

        updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
        updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);

        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    auto responseArea = getLocalBounds();

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
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

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

//==============================================================================
TradeMarkEQAudioProcessorEditor::TradeMarkEQAudioProcessorEditor(TradeMarkEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    lowPeakFreqSlider(*audioProcessor.apvts.getParameter("LowPeak Freq"), "Hz"),
    lowPeakGainSlider(*audioProcessor.apvts.getParameter("LowPeak Gain"), "dB"),
    lowPeakQualitySlider(*audioProcessor.apvts.getParameter("LowPeak Quality"), ""),
    midlowPeakFreqSlider(*audioProcessor.apvts.getParameter("MidLowPeak Freq"), "Hz"),
    midlowPeakGainSlider(*audioProcessor.apvts.getParameter("MidLowPeak Gain"), "dB"),
    midlowPeakQualitySlider(*audioProcessor.apvts.getParameter("MidLowPeak Quality"), ""),
    midPeakFreqSlider(*audioProcessor.apvts.getParameter("MidPeak Freq"), "Hz"),
    midPeakGainSlider(*audioProcessor.apvts.getParameter("MidPeak Gain"), "dB"),
    midPeakQualitySlider(*audioProcessor.apvts.getParameter("MidPeak Quality"), ""),
    midhighPeakFreqSlider(*audioProcessor.apvts.getParameter("MidHighPeak Freq"), "Hz"),
    midhighPeakGainSlider(*audioProcessor.apvts.getParameter("MidHighPeak Gain"), "dB"),
    midhighPeakQualitySlider(*audioProcessor.apvts.getParameter("MidHighPeak Quality"), ""),
    highPeakFreqSlider(*audioProcessor.apvts.getParameter("HighPeak Freq"), "Hz"),
    highPeakGainSlider(*audioProcessor.apvts.getParameter("HighPeak Gain"), "dB"),
    highPeakQualitySlider(*audioProcessor.apvts.getParameter("HighPeak Quality"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),

    responseCurveComponent(audioProcessor),
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
void TradeMarkEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

}

void TradeMarkEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    responseCurveComponent.setBounds(responseArea);

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
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}
