// Mini-synthé pédagogique — moteur sonore (une voix de polyphonie)
// © 2026 Mickael Le Van – tous droits réservés
// Même recette que la version web : oscillateur + bruit → filtre (avec son enveloppe) → ampli (avec son enveloppe).
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

// Valeurs des réglages, lues à chaque bloc (elles viennent du plugin ou d'un test)
struct VoiceParams
{
    std::atomic<float>* wave = nullptr;        // 0 sinus, 1 triangle, 2 dent de scie, 3 carré
    std::atomic<float>* oscLevel = nullptr;    // 0..1
    std::atomic<float>* detune = nullptr;      // cents
    std::atomic<float>* noiseLevel = nullptr;  // 0..1
    std::atomic<float>* noiseColor = nullptr;  // 0 blanc, 1 rose
    std::atomic<float>* filtType = nullptr;    // 0 passe-bas, 1 passe-haut, 2 passe-bande
    std::atomic<float>* cutoff = nullptr;      // Hz
    std::atomic<float>* res = nullptr;         // 0..1
    std::atomic<float>* envAmount = nullptr;   // octaves
    std::atomic<float>* fA = nullptr; std::atomic<float>* fD = nullptr; std::atomic<float>* fS = nullptr; std::atomic<float>* fR = nullptr;
    std::atomic<float>* aA = nullptr; std::atomic<float>* aD = nullptr; std::atomic<float>* aS = nullptr; std::atomic<float>* aR = nullptr;
};

struct MiniSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class MiniVoice : public juce::SynthesiserVoice
{
public:
    explicit MiniVoice (const VoiceParams& p) : P (p) {}

    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, maxBlock), 1 };
        filter.prepare (spec);
        filter.reset();
        ampEnv.setSampleRate (sampleRate);
        filtEnv.setSampleRate (sampleRate);
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<MiniSound*> (s) != nullptr; }

    void startNote (int midiNote, float, juce::SynthesiserSound*, int pitchWheel) override
    {
        note = midiNote;
        pitchWheelMoved (pitchWheel);
        phase = 0.0;
        pk0 = pk1 = pk2 = pk3 = pk4 = pk5 = pk6 = 0.0f;
        filter.reset();
        updateEnvelopes();
        ampEnv.reset();  filtEnv.reset();
        ampEnv.noteOn(); filtEnv.noteOn();
        counter = 0;
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) { ampEnv.noteOff(); filtEnv.noteOff(); }
        else { ampEnv.reset(); filtEnv.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int value) override { bend = (value - 8192) / 8192.0 * 2.0; } // ± 2 demi-tons
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! isVoiceActive()) return;
        updateEnvelopes();

        const int wave = (int) P.wave->load();
        const float oscLvl = P.oscLevel->load(), noiseLvl = P.noiseLevel->load();
        const bool pink = P.noiseColor->load() > 0.5f;
        const int type = (int) P.filtType->load();
        const float cutoff = P.cutoff->load(), amount = P.envAmount->load(), res = P.res->load();
        const double freq = 440.0 * std::pow (2.0, (note - 69) / 12.0 + P.detune->load() / 1200.0 + bend / 12.0);
        const double inc = freq / sr;

        // Résonance : mêmes réglages que la version web (Q en dB pour passe-bas/haut, Q linéaire pour passe-bande)
        const float q = type == 2 ? 0.5f * std::pow (60.0f, res) : 0.7071f * std::pow (10.0f, res * 24.0f / 20.0f);
        filter.setType (type == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                      : type == 1 ? juce::dsp::StateVariableTPTFilterType::highpass
                                  : juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setResonance (q);
        const float bpGain = type == 2 ? 1.0f / q : 1.0f;   // passe-bande ramené à 0 dB, comme sur le web
        const float maxFc = (float) juce::jmin (20000.0, sr * 0.45);

        for (int i = 0; i < num; ++i)
        {
            const float fe = filtEnv.getNextSample();
            const float ae = ampEnv.getNextSample();

            if ((counter++ & 7) == 0)
                filter.setCutoffFrequency (juce::jlimit (20.0f, maxFc, cutoff * std::pow (2.0f, amount * fe)));

            float s = oscLvl > 0.0f ? oscLvl * oscillator (wave, inc) : (advance (inc), 0.0f);
            if (noiseLvl > 0.0f) s += noiseLvl * noise (pink);

            const float y = filter.processSample (0, s) * bpGain * ae * peak;
            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, start + i, y);

            if (! ampEnv.isActive()) { clearCurrentNote(); filter.reset(); break; }
        }
    }

private:
    const VoiceParams& P;
    double sr = 48000.0, phase = 0.0, bend = 0.0;
    int note = 60;
    unsigned counter = 0;
    static constexpr float peak = 0.22f;   // volume d'une voix (comme la version web)
    juce::ADSR ampEnv, filtEnv;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::Random rng;
    float pk0 = 0, pk1 = 0, pk2 = 0, pk3 = 0, pk4 = 0, pk5 = 0, pk6 = 0;

    void updateEnvelopes()
    {
        ampEnv.setParameters  ({ P.aA->load(), P.aD->load(), P.aS->load(), P.aR->load() });
        filtEnv.setParameters ({ P.fA->load(), P.fD->load(), P.fS->load(), P.fR->load() });
    }

    void advance (double inc) { phase += inc; if (phase >= 1.0) phase -= 1.0; }

    // correction « polyBLEP » : adoucit les sauts de la dent de scie et du carré pour éviter les sons parasites
    static double polyBlep (double t, double dt)
    {
        if (t < dt)       { t /= dt;               return t + t - t * t - 1.0; }
        if (t > 1.0 - dt) { t = (t - 1.0) / dt;    return t * t + t + t + 1.0; }
        return 0.0;
    }

    float oscillator (int wave, double inc)
    {
        double v;
        switch (wave)
        {
            case 0:  v = std::sin (juce::MathConstants<double>::twoPi * phase); break;
            case 1:  v = phase < 0.25 ? 4.0 * phase : phase < 0.75 ? 2.0 - 4.0 * phase : 4.0 * phase - 4.0; break;
            case 2:  v = 2.0 * phase - 1.0 - polyBlep (phase, inc); break;
            default: { double p2 = phase + 0.5; if (p2 >= 1.0) p2 -= 1.0;
                       v = (phase < 0.5 ? 1.0 : -1.0) + polyBlep (phase, inc) - polyBlep (p2, inc); }
        }
        advance (inc);
        return (float) v;
    }

    float noise (bool pink)
    {
        const float w = rng.nextFloat() * 2.0f - 1.0f;
        if (! pink) return w * 0.5f;
        pk0 = 0.99886f * pk0 + w * 0.0555179f; pk1 = 0.99332f * pk1 + w * 0.0750759f; pk2 = 0.96900f * pk2 + w * 0.1538520f;
        pk3 = 0.86650f * pk3 + w * 0.3104856f; pk4 = 0.55000f * pk4 + w * 0.5329522f; pk5 = -0.7616f * pk5 - w * 0.0168980f;
        const float out = (pk0 + pk1 + pk2 + pk3 + pk4 + pk5 + pk6 + w * 0.5362f) * 0.12f;
        pk6 = w * 0.115926f;
        return out;
    }
};
