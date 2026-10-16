#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToyOrganSound::ToyOrganSound (ToyOrganAudioProcessor& proc)
    : processor (proc)
{
}

bool ToyOrganSound::appliesToNote (int midiNoteNumber)
{
    // Fallback synth only sounds if no custom sample is handling this note
    return ! processor.hasSampleForNote (midiNoteNumber);
}

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
        // A warm additive organ wave (fundamental + 2nd harmonic + 3rd harmonic)
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

    // 8 sampler voices + 8 synth fallback voices for polyphony
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new juce::SamplerVoice());
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new ToyOrganVoice());

    synth.addSound (new ToyOrganSound (*this));

    // Configure the 12 chromatic keys (C4 to B4: MIDI notes 60 to 71)
    const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    for (int i = 0; i < 12; ++i)
    {
        keySlots[(size_t) i].noteNumber = 60 + i;
        keySlots[(size_t) i].keyName = noteNames[i];
        keySlots[(size_t) i].isAccidental = (i == 1 || i == 3 || i == 6 || i == 8 || i == 10);
    }

    scanForSoundsets();
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

bool ToyOrganAudioProcessor::hasSampleForNote (int midiNoteNumber) const
{
    for (int i = 0; i < synth.getNumSounds(); ++i)
    {
        if (auto* samplerSound = dynamic_cast<juce::SamplerSound*> (synth.getSound (i).get()))
        {
            if (samplerSound->appliesToNote (midiNoteNumber))
                return true;
        }
    }
    return false;
}

bool ToyOrganAudioProcessor::isAnyKeyPressed() const
{
    for (int i = 0; i < 12; ++i)
    {
        if (keyboardState.isNoteOnForChannels (0xffff, keySlots[(size_t) i].noteNumber))
            return true;
    }
    return false;
}

int ToyOrganAudioProcessor::getCurrentlyPressedNote() const
{
    for (int i = 0; i < 12; ++i)
    {
        int note = keySlots[(size_t) i].noteNumber;
        if (keyboardState.isNoteOnForChannels (0xffff, note))
            return note;
    }
    return -1;
}

juce::File ToyOrganAudioProcessor::getSoundsetsFolder() const
{
    // 1. Check local repo path if running in dev environment
    juce::File devRepo ("toy-organ/Soundsets");
    if (devRepo.isDirectory())
        return devRepo;

    juce::File devScratch ("");
    if (devScratch.isDirectory())
        return devScratch;

    // 2. Next to executable
    auto execDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    auto nextToExec = execDir.getChildFile ("Soundsets");
    if (nextToExec.isDirectory())
        return nextToExec;

    // 3. User Music directory (default for installed plugin)
    auto userMusicDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("ToyOrgan/Soundsets");
    if (! userMusicDir.exists())
        userMusicDir.createDirectory();

    return userMusicDir;
}

void ToyOrganAudioProcessor::openSoundsetsFolder()
{
    auto folder = getSoundsetsFolder();
    if (! folder.exists())
        folder.createDirectory();

    folder.revealToUser();
}

void ToyOrganAudioProcessor::scanForSoundsets()
{
    detectedSoundsetFolders.clear();
    soundsetNames.clear();

    auto soundsetsDir = getSoundsetsFolder();
    if (! soundsetsDir.exists())
        soundsetsDir.createDirectory();

    auto subdirs = soundsetsDir.findChildFiles (juce::File::findDirectories, false);
    for (auto& dir : subdirs)
    {
        detectedSoundsetFolders.push_back (dir);
        soundsetNames.add (dir.getFileName());
    }

    if (soundsetNames.isEmpty())
    {
        soundsetNames.add ("Default Synth");
        currentSoundsetIndex = 0;
        currentSoundset.name = "Default Synth";
        currentSoundset.hasSamples = false;
        currentSoundset.sampleCount = 0;
        currentSoundset.mascotIdle = juce::Image();
        currentSoundset.mascotActive = juce::Image();
    }
    else
    {
        loadSoundsetByIndex (0);
    }
}

void ToyOrganAudioProcessor::loadSoundsetByIndex (int index)
{
    if (index >= 0 && index < (int) detectedSoundsetFolders.size())
    {
        currentSoundsetIndex = index;
        loadSoundsetFromFolder (detectedSoundsetFolders[(size_t) index]);
    }
}

void ToyOrganAudioProcessor::loadSoundsetFromFolder (const juce::File& folder)
{
    currentSoundset.name = folder.getFileName();
    currentSoundset.folder = folder;
    currentSoundset.hasSamples = false;
    currentSoundset.sampleCount = 0;
    currentSoundset.mascotIdle = juce::Image();
    currentSoundset.mascotActive = juce::Image();

    // Reset slot sample assignments
    for (auto& slot : keySlots)
        slot.sampleFile = juce::File();

    // 1. Gather files
    auto files = folder.findChildFiles (juce::File::findFiles, false);
    juce::Array<juce::File> imageFiles;
    juce::Array<juce::File> audioFiles;

    for (const auto& f : files)
    {
        auto ext = f.getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp")
            imageFiles.add (f);
        else if (ext == ".wav" || ext == ".aif" || ext == ".aiff" || ext == ".flac" || ext == ".ogg" || ext == ".mp3")
            audioFiles.add (f);
    }

    // 2. Identify Idle and Active Images
    juce::File idleFile;
    juce::File activeFile;

    for (const auto& img : imageFiles)
    {
        auto base = img.getFileNameWithoutExtension().toLowerCase();
        if (base.contains ("idle") || base.contains ("ruhe") || base.contains ("normal") || base.contains ("closed") || base == "1" || base == "01")
            idleFile = img;
        else if (base.contains ("active") || base.contains ("aktiv") || base.contains ("gaak") || base.contains ("sing") || base.contains ("open") || base == "2" || base == "02")
            activeFile = img;
    }

    if (! idleFile.existsAsFile() && imageFiles.size() > 0)
        idleFile = imageFiles[0];

    if (! activeFile.existsAsFile())
    {
        if (imageFiles.size() > 1 && imageFiles[1] != idleFile)
            activeFile = imageFiles[1];
        else if (idleFile.existsAsFile())
            activeFile = idleFile;
    }

    if (idleFile.existsAsFile())
        currentSoundset.mascotIdle = juce::ImageFileFormat::loadFrom (idleFile);

    if (activeFile.existsAsFile())
        currentSoundset.mascotActive = juce::ImageFileFormat::loadFrom (activeFile);

    // 3. Clear existing sounds and restore ToyOrganSound
    synth.clearSounds();
    synth.addSound (new ToyOrganSound (*this));

    // 4. Map Audio Samples
    if (audioFiles.size() == 1)
    {
        // Single sample pitched across the whole octave
        auto singleFile = audioFiles[0];
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (singleFile));
        if (reader != nullptr)
        {
            juce::BigInteger noteBits;
            noteBits.setRange (60, 12, true);
            synth.addSound (new juce::SamplerSound (
                singleFile.getFileNameWithoutExtension(),
                *reader,
                noteBits,
                60,   // Root note C4
                0.005,
                0.15,
                15.0
            ));
            currentSoundset.hasSamples = true;
            currentSoundset.sampleCount = 1;
            for (auto& slot : keySlots)
                slot.sampleFile = singleFile;
        }
    }
    else if (audioFiles.size() > 1)
    {
        int loadedCount = 0;
        const char* notePrefixes[12] = {
            "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"
        };
        const char* noteAltPrefixes[12] = {
            "c", "cs", "d", "ds", "e", "f", "fs", "g", "gs", "a", "as", "h"
        };

        for (int i = 0; i < 12; ++i)
        {
            juce::File matchedFile;
            juce::String indexStr = juce::String (i + 1);
            juce::String indexPadded = (i + 1 < 10 ? "0" : "") + indexStr;

            for (const auto& af : audioFiles)
            {
                auto base = af.getFileNameWithoutExtension().toLowerCase();
                if (base.startsWith (indexPadded) || base.startsWith (indexStr + "_") || base.startsWith (indexStr + "-"))
                {
                    matchedFile = af;
                    break;
                }
                if (base.contains (notePrefixes[i]) || base.contains (noteAltPrefixes[i]))
                {
                    matchedFile = af;
                    break;
                }
            }

            // Fallback: index-based if matching failed
            if (! matchedFile.existsAsFile() && i < audioFiles.size())
                matchedFile = audioFiles[i];

            if (matchedFile.existsAsFile())
            {
                std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (matchedFile));
                if (reader != nullptr)
                {
                    juce::BigInteger noteBits;
                    noteBits.setBit (60 + i);
                    synth.addSound (new juce::SamplerSound (
                        matchedFile.getFileNameWithoutExtension(),
                        *reader,
                        noteBits,
                        60 + i, // root note
                        0.005,
                        0.15,
                        15.0
                    ));
                    keySlots[(size_t) i].sampleFile = matchedFile;
                    loadedCount++;
                }
            }
        }

        if (loadedCount > 0)
        {
            currentSoundset.hasSamples = true;
            currentSoundset.sampleCount = loadedCount;
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToyOrganAudioProcessor();
}
