// Mini-synthé pédagogique — fenêtre du plugin : l'interface du site, intégrée dans une page web
// © 2026 Mickael Le Van – tous droits réservés
#pragma once
#include "PluginProcessor.h"

#if JUCE_WEB_BROWSER
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>

class MiniSynthEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MiniSynthEditor (MiniSynthProcessor&);
    ~MiniSynthEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MiniSynthProcessor& proc;
    std::unique_ptr<juce::WebBrowserComponent> web;
    std::map<juce::String, float> lastSent;
    juce::String dragging;
    bool pageReady = false;

    juce::dsp::FFT fft { 13 };                        // 8192 points
    juce::dsp::WindowingFunction<float> window { 8192, juce::dsp::WindowingFunction<float>::hann };
    std::vector<float> fftData = std::vector<float> (2 * 8192);

    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);
    juce::RangedAudioParameter* param (const juce::String& id) const;
    juce::DynamicObject::Ptr allParams();
    void sendFullState();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiniSynthEditor)
};
#endif
