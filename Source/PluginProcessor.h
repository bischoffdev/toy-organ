#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/** A simple toy-organ synth voice for immediate audio feedback before samples are loaded. */
class ToyOrganVoice : public juce::SynthesiserVoice
{
public:
    ToyOrganVoice() = default;

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote (float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved (int /*newPitchWheelValue*/) override {}
    void controllerMoved (int /*controllerNumber*/, int /*newControllerValue*/) override {}
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
    ToyOrganSound() = default;
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};

//==============================================================================
/** Represents one of the 12 organ keys and its sound/image configuration */
struct OrganKeySlot
{
    int noteNumber = 60; // 60 = Middle C (C4)
    juce::String keyName = "C";
    juce::File sampleFile;
    juce::Image idleImage;
    juce::Image activeImage;
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
    std::array<OrganKeySlot, 12>& getKeySlots() { return keySlots; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    // Helper for loading custom sound sets in future iterations
    void loadSampleForSlot (int slotIndex, const juce::File& audioFile);

private:
    juce::MidiKeyboardState keyboardState;
    juce::Synthesiser synth;
    juce::AudioFormatManager formatManager;

    std::array<OrganKeySlot, 12> keySlots;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToyOrganAudioProcessor)
};
