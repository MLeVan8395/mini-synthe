// Mini-synthé pédagogique — fenêtre du plugin
// © 2026 Mickael Le Van – tous droits réservés
#include "PluginEditor.h"

#if JUCE_WEB_BROWSER
#include "BinaryData.h"

namespace
{
    juce::WebBrowserComponent::Options makeOptions()
    {
        using O = juce::WebBrowserComponent::Options;
        auto folder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                          .getChildFile ("Mickael Le Van").getChildFile ("Mini-synthe").getChildFile ("WebView2");
        return O{}
            .withBackend (O::Backend::webview2)
            .withWinWebView2Options (O::WinWebView2{}
                                         .withUserDataFolder (folder)
                                         .withBackgroundColour (juce::Colour (0xff121419))
                                         .withStatusBarDisabled())
            .withNativeIntegrationEnabled()
            .withKeepPageLoadedWhenBrowserIsHidden();
    }

    juce::String intList (const float* data, int n, float scale)
    {
        juce::String s;
        s.preallocateBytes ((size_t) n * 6);
        for (int i = 0; i < n; ++i)
        {
            if (i) s << ',';
            s << juce::roundToInt (data[i] * scale);
        }
        return s;
    }
}

MiniSynthEditor::MiniSynthEditor (MiniSynthProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    auto opts = makeOptions()
        .withResourceProvider ([this] (const juce::String& url) { return getResource (url); })
        .withEventListener ("ready", [this] (const juce::var&) { pageReady = true; sendFullState(); })
        .withEventListener ("set", [this] (const juce::var& v)
        {
            const juce::String id = v["id"].toString();
            if (auto* rp = param (id))
            {
                const float value = (float) (double) v["value"];
                const bool inGesture = dragging == id;
                if (! inGesture) rp->beginChangeGesture();
                rp->setValueNotifyingHost (rp->convertTo0to1 (value));
                if (! inGesture) rp->endChangeGesture();
                lastSent[id] = rp->convertFrom0to1 (rp->getValue());
            }
        })
        .withEventListener ("gesture", [this] (const juce::var& v)
        {
            const juce::String id = v["id"].toString();
            if (auto* rp = param (id))
            {
                if ((bool) v["start"]) { if (dragging.isNotEmpty() && dragging != id) if (auto* o = param (dragging)) o->endChangeGesture();
                                         dragging = id; rp->beginChangeGesture(); }
                else if (dragging == id) { rp->endChangeGesture(); dragging.clear(); }
            }
        })
        .withEventListener ("program", [this] (const juce::var& v)
        {
            proc.setCurrentProgram ((int) v["index"]);
            proc.updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
        })
        .withEventListener ("note", [this] (const juce::var& v)
        {
            const int n = juce::jlimit (0, 127, (int) v["n"]);
            if ((bool) v["on"]) proc.keyboardState.noteOn (1, n, 0.8f);
            else                proc.keyboardState.noteOff (1, n, 0.0f);
        });

    web = std::make_unique<juce::WebBrowserComponent> (opts);
    addAndMakeVisible (*web);
    web->goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setResizable (true, true);
    setResizeLimits (720, 520, 1800, 1400);
    setSize (1080, 820);
    startTimerHz (30);
}

MiniSynthEditor::~MiniSynthEditor()
{
    stopTimer();
    if (dragging.isNotEmpty()) if (auto* rp = param (dragging)) rp->endChangeGesture();
    proc.keyboardState.allNotesOff (1);
}

void MiniSynthEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff121419)); }
void MiniSynthEditor::resized() { web->setBounds (getLocalBounds()); }

std::optional<juce::WebBrowserComponent::Resource> MiniSynthEditor::getResource (const juce::String& url)
{
    if (url == "/" || url == "/index.html")
    {
        const auto* d = reinterpret_cast<const std::byte*> (BinaryData::index_html);
        return juce::WebBrowserComponent::Resource { std::vector<std::byte> (d, d + BinaryData::index_htmlSize), "text/html" };
    }
    return std::nullopt;
}

juce::RangedAudioParameter* MiniSynthEditor::param (const juce::String& id) const
{
    return proc.apvts.getParameter (id);
}

juce::DynamicObject::Ptr MiniSynthEditor::allParams()
{
    juce::DynamicObject::Ptr o = new juce::DynamicObject();
    for (auto* p : proc.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            const float v = rp->convertFrom0to1 (rp->getValue());
            o->setProperty (rp->getParameterID(), v);
            lastSent[rp->getParameterID()] = v;
        }
    return o;
}

void MiniSynthEditor::sendFullState()
{
    juce::DynamicObject::Ptr st = new juce::DynamicObject();
    st->setProperty ("params", juce::var (allParams().get()));
    juce::Array<juce::var> names;
    for (int i = 0; i < proc.getNumPrograms(); ++i) names.add (proc.getProgramName (i));
    st->setProperty ("presets", names);
    st->setProperty ("program", proc.getCurrentProgram());
    web->emitEventIfBrowserIsVisible ("state", juce::var (st.get()));
}

void MiniSynthEditor::timerCallback()
{
    if (! pageReady) return;

    // 1. réglages changés ailleurs (automation, sons de départ, bouton MIDI du logiciel…)
    juce::DynamicObject::Ptr changed = new juce::DynamicObject();
    for (auto* p : proc.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            const auto id = rp->getParameterID();
            if (id == dragging) continue;
            const float v = rp->convertFrom0to1 (rp->getValue());
            auto it = lastSent.find (id);
            if (it == lastSent.end() || std::abs (it->second - v) > 1e-6f * (1.0f + std::abs (v)))
            {
                changed->setProperty (id, v);
                lastSent[id] = v;
            }
        }
    if (! changed->getProperties().isEmpty())
        web->emitEventIfBrowserIsVisible ("params", juce::var (changed.get()));

    // 2. l'écran : forme d'onde (calée sur la note jouée), spectre, enveloppes, touches enfoncées
    constexpr int S = MiniSynthProcessor::scopeSize;
    std::vector<float> recent ((size_t) S);
    const int w = proc.scopeWrite.load();
    for (int i = 0; i < S; ++i) recent[(size_t) i] = proc.scopeBuffer[(size_t) ((w + i) & (S - 1))];

    juce::DynamicObject::Ptr v = new juce::DynamicObject();
    auto* voice = proc.latestVoice();
    const bool active = voice != nullptr && voice->vizActive.load();
    const int note = voice != nullptr ? voice->vizNote.load() : -1;

    const double sr = proc.getSampleRate() > 0 ? proc.getSampleRate() : 48000.0;
    const double f = note >= 0 ? 440.0 * std::pow (2.0, (note - 69) / 12.0) : 220.0;
    const int period = (int) (sr / f);
    const int N = juce::jlimit (250, 3000, period * 3);
    int start = S - N - period - 1;            // on cherche un passage par zéro « montant » pour stabiliser l'image
    for (int i = start; i > 1 && i > start - period - 2; --i)
        if (recent[(size_t) i - 1] < 0.0f && recent[(size_t) i] >= 0.0f) { start = i; break; }
    std::vector<float> shown ((size_t) N);
    const int step = N > 600 ? N / 600 + 1 : 1;   // pas plus de 600 points envoyés
    int k = 0;
    for (int i = 0; i < N; i += step) shown[(size_t) k++] = recent[(size_t) (start + i)];
    v->setProperty ("w", intList (shown.data(), k, 1000.0f));

    std::fill (fftData.begin(), fftData.end(), 0.0f);
    std::copy (recent.begin(), recent.end(), fftData.begin());
    window.multiplyWithWindowingTable (fftData.data(), 8192);
    fft.performFrequencyOnlyForwardTransform (fftData.data());
    constexpr int bands = 300;
    float spec[bands];
    const double binHz = sr / 8192.0;
    for (int b = 0; b < bands; ++b)
    {
        const double f1 = 20.0 * std::pow (1000.0, b / (double) bands), f2 = 20.0 * std::pow (1000.0, (b + 1) / (double) bands);
        int i1 = juce::jlimit (1, 4095, (int) std::round (f1 / binHz)), i2 = juce::jlimit (i1, 4095, (int) std::round (f2 / binHz));
        float m = 0.0f;
        for (int i = i1; i <= i2; ++i) m = juce::jmax (m, fftData[(size_t) i]);
        const float db = juce::Decibels::gainToDecibels (m * 4.0f / 8192.0f, -200.0f);
        spec[b] = juce::jlimit (0.0f, 1.0f, (db + 100.0f) / 85.0f);
    }
    v->setProperty ("s", intList (spec, bands, 1000.0f));

    v->setProperty ("a", active);
    v->setProperty ("n", active || (voice != nullptr && voice->vizSerial.load() > 0) ? note : -1);
    if (voice != nullptr)
    {
        v->setProperty ("t", voice->vizAge.load());
        v->setProperty ("r", voice->vizReleased.load());
        v->setProperty ("rt", voice->vizRelAge.load());
        v->setProperty ("ea", voice->vizAmp.load());
        v->setProperty ("ef", voice->vizFilt.load());
    }
    juce::String keys;
    for (int n = 0; n < 128; ++n)
        if (proc.keyboardState.isNoteOnForChannels (0xffff, n)) keys << (keys.isEmpty() ? "" : ",") << n;
    v->setProperty ("k", keys);
    web->emitEventIfBrowserIsVisible ("viz", juce::var (v.get()));
}
#endif
