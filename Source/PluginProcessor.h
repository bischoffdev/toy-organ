#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <array>

class ToyOrganAudioProcessor;

//==============================================================================
/** A simple toy-organ synth voice for immediate audio feedback when samples are not loaded. */
class ToyOrganVoice : public juce::SynthesiserVoice
{
public:
    ToyOrganVoice() = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    float level = 0.0f;
    float tailOff = 0.0f;
};

//==============================================================================
class ToyOrganSound : public juce::SynthesiserSound
{
public:
    explicit ToyOrganSound (ToyOrganAudioProcessor& proc);
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override { juce::ignoreUnused (midiChannel); return true; }

private:
    ToyOrganAudioProcessor& processor;
};

//==============================================================================
/** Represents one of the 12 organ keys */
struct OrganKeySlot
{
    int noteNumber = 60; // 60 = Middle C (C4)
    juce::String keyName = "C";
    bool isAccidental = false;
    juce::File sampleFile;
};

//==============================================================================
/** Represents an active soundset with mascot pictures and sample files */
struct SoundsetInfo
{
    juce::String name;
    juce::File folder;
    juce::Image mascotIdle;
    juce::Image mascotActive;
    juce::Colour keyColor { 0xFFFFEAA7 };
    bool hasSamples = false;
    int sampleCount = 0;
};

//==============================================================================
class ToyOrganAudioProcessor : public juce::AudioProcessor
{
public:
    ToyOrganAudioProcessor();
    ~ToyOrganAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    std::array<OrganKeySlot, 13>& getKeySlots() { return keySlots; }
    const std::array<OrganKeySlot, 13>& getKeySlots() const { return keySlots; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    bool hasSampleForNote (int midiNoteNumber) const;
    bool isAnyKeyPressed() const;
    int getCurrentlyPressedNote() const;

    // Soundset management
    void scanForSoundsets();
    juce::StringArray getSoundsetNames() const { return soundsetNames; }
    int getCurrentSoundsetIndex() const { return currentSoundsetIndex; }
    void loadSoundsetByIndex (int index);
    const SoundsetInfo& getCurrentSoundset() const { return currentSoundset; }
    juce::File getSoundsetsFolder() const;
    void openSoundsetsFolder();

private:
    juce::MidiKeyboardState keyboardState;
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;

    std::array<OrganKeySlot, 13> keySlots;
    std::vector<juce::File> detectedSoundsetFolders;
    juce::StringArray soundsetNames;
    int currentSoundsetIndex = 0;
    SoundsetInfo currentSoundset;

    void loadSoundsetFromFolder (const juce::File& folder);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessor)
};
