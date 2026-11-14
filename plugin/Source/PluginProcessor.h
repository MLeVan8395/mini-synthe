// Mini-synthé pédagogique — plugin VST3
// © 2026 Mickael Le Van – tous droits réservés
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "SynthVoice.h"
#include <array>

class MiniSynthProcessor : public juce::AudioProcessor
{
public:
    MiniSynthProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Pour l'interface : clavier à l'écran, derniers instants du son, voix la plus récente
    juce::MidiKeyboardState keyboardState;
    static constexpr int scopeSize = 8192;
    std::array<float, scopeSize> scopeBuffer {};
    std::atomic<int> scopeWrite { 0 };
    MiniVoice* latestVoice() const;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    VoiceParams vp;
    juce::Synthesiser synth;
    std::atomic<float>* master = nullptr;
    juce::SmoothedValue<float> masterSmoothed;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiniSynthProcessor)
};
