// Mini-synthé pédagogique — plugin VST3
// © 2026 Mickael Le Van – tous droits réservés
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::String T (const char* s) { return juce::String::fromUTF8 (s); }

    juce::StringArray TA (std::initializer_list<const char*> items)
    {
        juce::StringArray a;
        for (auto* s : items) a.add (T (s));
        return a;
    }

    // curseur à échelle logarithmique (comme sur la version web) : le milieu du curseur = moyenne géométrique
    juce::NormalisableRange<float> logRange (float lo, float hi)
    {
        return { lo, hi,
                 [] (float a, float b, float x) { return a * std::pow (b / a, x); },
                 [] (float a, float b, float v) { return std::log (v / a) / std::log (b / a); },
                 [] (float a, float b, float v) { return juce::jlimit (a, b, v); } };
    }

    // nombres à la française : virgule décimale
    juce::String dec (float v, int places) { return juce::String (v, places).replaceCharacter ('.', ','); }

    juce::String fmtSeconds (float v, int) { return v < 1.0f ? juce::String (juce::roundToInt (v * 1000.0f)) + " ms" : dec (v, 2) + " s"; }
    juce::String fmtPct (float v, int)     { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; }
    juce::String fmtHz (float v, int)      { return v >= 1000.0f ? dec (v / 1000.0f, 1) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz"; }
    juce::String fmtOct (float v, int)     { return (v > 0 ? "+" : "") + dec (v, 1) + " oct."; }
    juce::String fmtCents (float v, int)   { return (v > 0 ? "+" : "") + juce::String (juce::roundToInt (v)) + " cents"; }

    // Sons de départ (mêmes que sur le site, pour ceux qui n'utilisent pas d'effets)
    struct Preset { const char* name; std::vector<std::pair<const char*, float>> values; };
    const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> list = {
            { "Init (dent de scie)", {} },
            { "Son pur",      { {"wave",0}, {"cutoff",18000}, {"res",0}, {"envAmount",0}, {"aA",0.01f}, {"aD",0.1f}, {"aS",1}, {"aR",0.15f} } },
            { "Orgue",        { {"wave",3}, {"cutoff",3500}, {"res",0}, {"envAmount",0}, {"aA",0.005f}, {"aD",0.05f}, {"aS",1}, {"aR",0.05f} } },
            { "Basse synthé", { {"wave",2}, {"cutoff",180}, {"res",0.55f}, {"envAmount",3.6f}, {"fA",0.003f}, {"fD",0.22f}, {"fS",0.1f}, {"fR",0.15f}, {"aA",0.003f}, {"aD",0.4f}, {"aS",0.8f}, {"aR",0.1f} } },
            { "Son pincé",    { {"wave",2}, {"cutoff",250}, {"res",0.2f}, {"envAmount",4.2f}, {"fA",0.001f}, {"fD",0.25f}, {"fS",0}, {"fR",0.2f}, {"aA",0.001f}, {"aD",0.7f}, {"aS",0}, {"aR",0.3f} } },
            { "Cuivre",       { {"wave",2}, {"cutoff",450}, {"res",0.1f}, {"envAmount",2.6f}, {"fA",0.09f}, {"fD",0.5f}, {"fS",0.55f}, {"fR",0.2f}, {"aA",0.04f}, {"aD",0.3f}, {"aS",0.9f}, {"aR",0.2f} } },
            { "Nappe (pad)",  { {"wave",2}, {"detune",-6}, {"cutoff",700}, {"res",0.1f}, {"envAmount",1.8f}, {"fA",1.6f}, {"fD",2}, {"fS",0.5f}, {"fR",2.5f}, {"aA",1.2f}, {"aD",1}, {"aS",0.85f}, {"aR",2.8f} } },
            { "Charleston",   { {"oscLevel",0}, {"noiseLevel",1}, {"noiseColor",0}, {"filtType",1}, {"cutoff",7000}, {"res",0.2f}, {"envAmount",0}, {"aA",0.001f}, {"aD",0.07f}, {"aS",0}, {"aR",0.06f} } },
            { "Vent",         { {"oscLevel",0}, {"noiseLevel",1}, {"noiseColor",1}, {"filtType",2}, {"cutoff",700}, {"res",0.7f}, {"envAmount",1}, {"fA",1.5f}, {"fD",2}, {"fS",0.3f}, {"fR",2}, {"aA",1}, {"aD",0.5f}, {"aS",0.9f}, {"aR",2} } },
        };
        return list;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MiniSynthProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout L;
    auto pid = [] (const char* id) { return ParameterID { id, 1 }; };

    auto fl = [&] (const char* id, const char* name, NormalisableRange<float> r, float def, String (*fmt) (float, int), const char* unit = "")
    {
        L.add (std::make_unique<AudioParameterFloat> (pid (id), T (name), r, def,
                   AudioParameterFloatAttributes().withLabel (T (unit)).withStringFromValueFunction (fmt)));
    };
    auto ch = [&] (const char* id, const char* name, StringArray items, int def)
    {
        L.add (std::make_unique<AudioParameterChoice> (pid (id), T (name), items, def));
    };

    // 1 — Oscillateur et bruit
    ch ("wave", "1 Oscillateur · Forme d'onde", TA ({ "Sinus", "Triangle", "Dent de scie", "Carré" }), 2);
    fl ("oscLevel", "1 Oscillateur · Niveau", { 0.0f, 1.0f }, 0.8f, fmtPct);
    fl ("detune", "1 Oscillateur · Accord fin", { -100.0f, 100.0f, 1.0f }, 0.0f, fmtCents);
    ch ("noiseColor", "1 Bruit · Couleur", TA ({ "Bruit blanc", "Bruit rose" }), 0);
    fl ("noiseLevel", "1 Bruit · Niveau", { 0.0f, 1.0f }, 0.0f, fmtPct);
    // 2 — Filtre et son enveloppe
    ch ("filtType", "2 Filtre · Type", TA ({ "Passe-bas", "Passe-haut", "Passe-bande" }), 0);
    fl ("cutoff", "2 Filtre · Fréquence de coupure", logRange (40.0f, 18000.0f), 2500.0f, fmtHz);
    fl ("res", "2 Filtre · Résonance", { 0.0f, 1.0f }, 0.15f, fmtPct);
    fl ("envAmount", "2 Filtre · Quantité d'enveloppe", { -4.0f, 6.0f, 0.1f }, 1.5f, fmtOct);
    fl ("fA", "2 Filtre · A Attaque", logRange (0.001f, 4.0f), 0.01f, fmtSeconds);
    fl ("fD", "2 Filtre · D Déclin", logRange (0.005f, 4.0f), 0.4f, fmtSeconds);
    fl ("fS", "2 Filtre · S Maintien", { 0.0f, 1.0f }, 0.4f, fmtPct);
    fl ("fR", "2 Filtre · R Relâchement", logRange (0.005f, 6.0f), 0.4f, fmtSeconds);
    // 3 — Ampli et son enveloppe
    fl ("aA", "3 Ampli · A Attaque", logRange (0.001f, 4.0f), 0.01f, fmtSeconds);
    fl ("aD", "3 Ampli · D Déclin", logRange (0.005f, 4.0f), 0.3f, fmtSeconds);
    fl ("aS", "3 Ampli · S Maintien", { 0.0f, 1.0f }, 0.7f, fmtPct);
    fl ("aR", "3 Ampli · R Relâchement", logRange (0.005f, 6.0f), 0.4f, fmtSeconds);
    // Sortie
    fl ("master", "Volume général", { 0.0f, 1.0f }, 0.7f, fmtPct);
    return L;
}

MiniSynthProcessor::MiniSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Sortie", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MiniSynthe", createLayout())
{
    auto p = [this] (const char* id) { return apvts.getRawParameterValue (id); };
    vp.wave = p ("wave"); vp.oscLevel = p ("oscLevel"); vp.detune = p ("detune");
    vp.noiseLevel = p ("noiseLevel"); vp.noiseColor = p ("noiseColor");
    vp.filtType = p ("filtType"); vp.cutoff = p ("cutoff"); vp.res = p ("res"); vp.envAmount = p ("envAmount");
    vp.fA = p ("fA"); vp.fD = p ("fD"); vp.fS = p ("fS"); vp.fR = p ("fR");
    vp.aA = p ("aA"); vp.aD = p ("aD"); vp.aS = p ("aS"); vp.aR = p ("aR");
    master = p ("master");

    for (int i = 0; i < 16; ++i) synth.addVoice (new MiniVoice (vp));
    synth.addSound (new MiniSound());
}

void MiniSynthProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<MiniVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlock);
    masterSmoothed.reset (sampleRate, 0.02);
    masterSmoothed.setCurrentAndTargetValue (master->load());
}

bool MiniSynthProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void MiniSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);   // notes du clavier à l'écran
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    // volume général, puis un adoucisseur qui évite la saturation brutale quand on joue beaucoup de notes
    masterSmoothed.setTargetValue (master->load());
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float g = masterSmoothed.getNextValue();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float x = buffer.getSample (ch, i) * g;
            const float a = std::abs (x);
            if (a > 0.8f) x = std::copysign (0.8f + 0.2f * std::tanh ((a - 0.8f) / 0.2f), x);
            buffer.setSample (ch, i, x);
        }
    }

    // on garde les derniers instants du son pour l'écran (forme d'onde, spectre)
    int w = scopeWrite.load();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        scopeBuffer[(size_t) w] = buffer.getSample (0, i);
        w = (w + 1) & (scopeSize - 1);
    }
    scopeWrite.store (w);
}

MiniVoice* MiniSynthProcessor::latestVoice() const
{
    MiniVoice* best = nullptr;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<MiniVoice*> (synth.getVoice (i)))
            if (best == nullptr || v->vizSerial.load() > best->vizSerial.load())
                best = v;
    return best;
}

int MiniSynthProcessor::getNumPrograms() { return (int) presets().size(); }
const juce::String MiniSynthProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, (int) presets().size()) ? T (presets()[(size_t) index].name) : juce::String();
}
void MiniSynthProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) presets().size())) return;
    currentProgram = index;
    for (auto* param : getParameters())   // on repart des valeurs par défaut…
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (param))
            rp->setValueNotifyingHost (rp->getDefaultValue());
    for (auto& [id, value] : presets()[(size_t) index].values)   // …puis on applique le son de départ
        if (auto* rp = apvts.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
}

juce::AudioProcessorEditor* MiniSynthProcessor::createEditor()
{
   #if JUCE_WEB_BROWSER
    return new MiniSynthEditor (*this);          // l'interface pédagogique (page web intégrée)
   #else
    return new juce::GenericAudioProcessorEditor (*this);   // secours : simple liste de curseurs
   #endif
}

void MiniSynthProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MiniSynthProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MiniSynthProcessor(); }
