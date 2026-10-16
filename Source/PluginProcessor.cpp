#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
bool ToyOrganVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<ToyOrganSound*> (sound) != nullptr;
}

void ToyOrganVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentAngle = 0.0;
    level = velocity * 0.25f;
    tailOff = 0.0f;

    auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void ToyOrganVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0f)
            tailOff = 1.0f;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;
    }
}

void ToyOrganVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta == 0.0)
        return;

    while (--numSamples >= 0)
    {
        // A warm additive organ-style wave (fundamental + 2nd harmonic + 3rd harmonic)
        auto sample = (float) (std::sin (currentAngle) 
                             + 0.5 * std::sin (2.0 * currentAngle) 
                             + 0.25 * std::sin (3.0 * currentAngle)) * level;

        if (tailOff > 0.0f)
        {
            sample *= tailOff;
            tailOff *= 0.99f;

            if (tailOff <= 0.005f)
            {
                clearCurrentNote();
                angleDelta = 0.0;
                break;
            }
        }

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample (channel, startSample, sample);

        currentAngle += angleDelta;
        ++startSample;
    }
}

//==============================================================================
ToyOrganAudioProcessor::ToyOrganAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();

    // 8-voice polyphony for the organ
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new ToyOrganVoice());

    synth.addSound (new ToyOrganSound());

    // Configure the 12 keys (C4 to B4: MIDI notes 60 to 71)
    const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
    {
        keySlots[i].noteNumber = 60 + i;
        keySlots[i].keyName = noteNames[i];
    }
}

ToyOrganAudioProcessor::~ToyOrganAudioProcessor()
{
}

const juce::String ToyOrganAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ToyOrganAudioProcessor::acceptsMidi() const
{
    return true;
}

bool ToyOrganAudioProcessor::producesMidi() const
{
    return false;
}

bool ToyOrganAudioProcessor::isMidiEffect() const
{
    return false;
}

double ToyOrganAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ToyOrganAudioProcessor::getNumPrograms()
{
    return 1;
}

int ToyOrganAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ToyOrganAudioProcessor::setCurrentProgram (int)
{
}

const juce::String ToyOrganAudioProcessor::getProgramName (int)
{
    return {};
}

void ToyOrganAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void ToyOrganAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    keyboardState.reset();
    juce::ignoreUnused (samplesPerBlock);
}

void ToyOrganAudioProcessor::releaseResources()
{
    keyboardState.reset();
}

bool ToyOrganAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Output must be mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void ToyOrganAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Connect GUI / on-screen key presses to the audio MIDI stream
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Render sound
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

bool ToyOrganAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ToyOrganAudioProcessor::createEditor()
{
    return new ToyOrganAudioProcessorEditor (*this);
}

void ToyOrganAudioProcessor::getStateInformation (juce::MemoryBlock&)
{
}

void ToyOrganAudioProcessor::setStateInformation (const void*, int)
{
}

void ToyOrganAudioProcessor::loadSampleForSlot (int slotIndex, const juce::File& audioFile)
{
    if (juce::isPositiveAndBelow (slotIndex, 12) && audioFile.existsAsFile())
    {
        keySlots[slotIndex].sampleFile = audioFile;
        // In subsequent iterations, sample decoding and SamplerVoice integration connects here!
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToyOrganAudioProcessor();
}
