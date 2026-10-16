#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
/** Interactive visual representation of an organ key with its idle/active avatar above */
class ToyOrganKeyView : public juce::Component
{
public:
    ToyOrganKeyView (int slotIndex, ToyOrganAudioProcessor& processor);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void setIsPressed (bool pressed);

private:
    int slotIndex;
    ToyOrganAudioProcessor& processor;
    bool isPressed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganKeyView)
};

//==============================================================================
class ToyOrganAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::MidiKeyboardState::Listener,
                                     public juce::Timer
{
public:
    explicit ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor&);
    ~ToyOrganAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // MidiKeyboardState::Listener callbacks to animate on MIDI input
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    void timerCallback() override;

private:
    ToyOrganAudioProcessor& audioProcessor;

    std::vector<std::unique_ptr<ToyOrganKeyView>> keyViews;
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessorEditor)
};
