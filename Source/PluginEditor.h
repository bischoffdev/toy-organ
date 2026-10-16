#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
/** Interactive visual representation of an organ key (White or Black/Halbton) */
class ToyOrganKeyView : public juce::Component
{
public:
    ToyOrganKeyView (int slotIndex, bool isAccidental, ToyOrganAudioProcessor& processor);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    void setIsPressed (bool pressed);
    bool getIsPressed() const { return isPressed; }
    int getSlotIndex() const { return slotIndex; }
    bool getIsAccidental() const { return isAccidental; }

private:
    int slotIndex;
    bool isAccidental;
    ToyOrganAudioProcessor& processor;
    bool isPressed = false;

    void triggerNoteOn();
    void triggerNoteOff();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganKeyView)
};

//==============================================================================
/** Mascot display stage showing Idle / Active pictures (e.g. Das Huhn) */
class ToyOrganMascotView : public juce::Component
{
public:
    explicit ToyOrganMascotView (ToyOrganAudioProcessor& processor);

    void paint (juce::Graphics& g) override;

private:
    ToyOrganAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganMascotView)
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

    // MidiKeyboardState::Listener callbacks
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    void timerCallback() override;

private:
    ToyOrganAudioProcessor& audioProcessor;

    // Visual subcomponents
    std::vector<std::unique_ptr<ToyOrganKeyView>> keyViews;
    ToyOrganMascotView mascotView;

    // Header controls
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label soundsetLabel;
    juce::ComboBox soundsetComboBox;
    juce::TextButton openFolderButton;
    juce::TextButton refreshButton;

    void populateSoundsets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessorEditor)
};
