# Mini-synthé pédagogique — plugin VST3 (et version autonome)
# © 2026 Mickael Le Van – tous droits réservés
cmake_minimum_required(VERSION 3.22)
project(MINI_SYNTHE VERSION 0.1.0)

# JUCE : dossier local (-DJUCE_DIR=...) ou téléchargé automatiquement
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.15 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(MiniSynthe
    COMPANY_NAME "Mickael Le Van"
    PRODUCT_NAME "Mini-synthe"
    PLUGIN_MANUFACTURER_CODE Mlvn
    PLUGIN_CODE Msyn
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST3_CATEGORIES Instrument Synth
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_compile_features(MiniSynthe PRIVATE cxx_std_17)
target_sources(MiniSynthe PRIVATE Source/PluginProcessor.cpp)

target_compile_definitions(MiniSynthe PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if(MSVC)
    target_compile_options(MiniSynthe PRIVATE /utf-8)
endif()

target_link_libraries(MiniSynthe
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
