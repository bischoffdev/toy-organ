#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================================
/** Clean LookAndFeel for toy organ controls (light on dark) */
class ToyOrganLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ToyOrganLookAndFeel();

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    juce::Font getComboBoxFont (juce::ComboBox& box) override;
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};

//==============================================================================
/** Vector icon button for header toolbar (crisp, resolution-independent) */
class ToyOrganIconButton : public juce::Button
{
public:
    enum class IconType { Prev, Next, Folder, Reload };

    ToyOrganIconButton (IconType type, const juce::String& tooltipText = {});

    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    IconType iconType;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganIconButton)
};

//==============================================================================
/** Interactive visual mascot picture view */
class ToyOrganKeyView : public juce::Component
{
public:
    ToyOrganKeyView (int slotIndex, bool isAccidental, ToyOrganAudioProcessor& processor);

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    void setIsPressed (bool pressed);
    bool getIsPressed() const { return isPressed; }
    int getSlotIndex() const { return slotIndex; }
    bool getIsAccidental() const { return isAccidental; }
    void updateAnimation (int animTick);

private:
    int slotIndex;
    bool isAccidental;
    ToyOrganAudioProcessor& processor;
    bool isPressed = false;
    int pressTicks = 0;
    int globalAnimTick = 0;

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

    // Musical typing via computer keyboard
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void focusGained (FocusChangeType cause) override;
    void focusLost (FocusChangeType cause) override;

private:
    ToyOrganAudioProcessor& audioProcessor;
    ToyOrganLookAndFeel customLookAndFeel;

    // Visual subcomponents (13 picture views: C4 to C5 arranged in 2 tiered rows)
    std::vector<std::unique_ptr<ToyOrganKeyView>> keyViews;

    // Logo image at top left
    juce::Image logoImage;

    // Header controls - Left
    juce::Label titleLabel;

    // Header controls - Center Soundset Browser
    ToyOrganIconButton prevSoundsetButton { ToyOrganIconButton::IconType::Prev, "Vorheriges Soundset" };
    juce::ComboBox soundsetComboBox;
    ToyOrganIconButton nextSoundsetButton { ToyOrganIconButton::IconType::Next, "Nächstes Soundset" };
    ToyOrganIconButton openFolderButton   { ToyOrganIconButton::IconType::Folder, "Soundsets-Ordner öffnen" };

    // Header controls - Right
    juce::ComboBox layoutComboBox;
    juce::Label volumeLabel;
    juce::Slider volumeSlider;
    juce::Label volValueLabel;

    void populateSoundsets();
    void updateComputerKeyboardNotes();

    bool computerKeyHeld[13] = { false };
    int animTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessorEditor)
};
