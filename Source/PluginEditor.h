#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
/** Interactive visual representation of an organ key with mascot character */
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
    void drawVectorMascot (juce::Graphics& g, juce::Rectangle<float> area, bool pressed);

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

    // MidiKeyboardState::Listener callbacks
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    void timerCallback() override;

private:
    ToyOrganAudioProcessor& audioProcessor;

    // Visual subcomponents (13 keys: C4 to C5)
    std::vector<std::unique_ptr<ToyOrganKeyView>> keyViews;

    // Header controls
    juce::Label titleLabel;
    juce::Label soundsetLabel;
    juce::ComboBox soundsetComboBox;
    juce::TextButton openFolderButton;
    juce::TextButton refreshButton;

    void populateSoundsets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessorEditor)
};

