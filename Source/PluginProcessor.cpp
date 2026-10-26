#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToyOrganSound::ToyOrganSound (ToyOrganAudioProcessor& proc)
    : processor (proc)
{
}

bool ToyOrganSound::appliesToNote (int midiNoteNumber)
{
    // Strictly ONLY support the organ keys (MIDI 60 to 72 = C4 to C5)
    if (midiNoteNumber < 60 || midiNoteNumber > 72)
        return false;

    // Fallback synth only sounds if no custom sample is handling this note
    return ! processor.hasSampleForNote (midiNoteNumber);
}

//==============================================================================
bool ToyOrganVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<ToyOrganSound*> (sound) != nullptr;
}

void ToyOrganVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    if (midiNoteNumber < 60 || midiNoteNumber > 72)
        return;

    currentAngle = 0.0;
    level = velocity * 0.25f;
    tailOff = 0.0f;
    lfoPhase = 0.0;

    auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    auto cyclesPerSample = cyclesPerSecond / getSampleRate();
    baseAngleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    angleDelta = baseAngleDelta;

    pitchWheelMoved (currentPitchWheelPosition);
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
        baseAngleDelta = 0.0;
        angleDelta = 0.0;
    }
}

void ToyOrganVoice::pitchWheelMoved (int newValue)
{
    pitchBendRatio = std::pow (2.0, (double) (newValue - 8192) / (8192.0 * 6.0));
}

void ToyOrganVoice::controllerMoved (int controllerNumber, int newValue)
{
    if (controllerNumber == 1) // Modulation Wheel
        vibratoDepth = (float) newValue / 127.0f;
}

void ToyOrganVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (baseAngleDelta == 0.0)
        return;

    const double lfoDelta = (2.0 * juce::MathConstants<double>::pi * 5.5) / getSampleRate();
    const double maxVibratoSemitones = 0.6;

    while (--numSamples >= 0)
    {
        double vibratoRatio = 1.0;
        if (vibratoDepth > 0.001f)
        {
            double semitones = std::sin (lfoPhase) * (vibratoDepth * maxVibratoSemitones);
            vibratoRatio = std::pow (2.0, semitones / 12.0);
            lfoPhase += lfoDelta;
            if (lfoPhase >= 2.0 * juce::MathConstants<double>::pi)
                lfoPhase -= 2.0 * juce::MathConstants<double>::pi;
        }

        angleDelta = baseAngleDelta * pitchBendRatio * vibratoRatio;

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
                baseAngleDelta = 0.0;
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
ToyOrganSampledSound::ToyOrganSampledSound (const juce::String& soundName,
                                            juce::AudioFormatReader& reader,
                                            const juce::BigInteger& notes,
                                            int midiRootNote,
                                            double attackTimeSecs,
                                            double releaseTimeSecs,
                                            double maxSampleLengthSeconds)
    : name (soundName), midiNotes (notes), rootNote (midiRootNote), sourceSampleRate (reader.sampleRate)
{
    if (sourceSampleRate > 0.0)
    {
        length = juce::jmin ((int) reader.lengthInSamples,
                             (int) (maxSampleLengthSeconds * sourceSampleRate));
        if (length > 0)
        {
            data.setSize ((int) reader.numChannels, length + 4);
            data.clear();
            reader.read (&data, 0, length, 0, true, true);
        }
    }
    params.attack = (float) attackTimeSecs;
    params.decay = 0.1f;
    params.sustain = 1.0f;
    params.release = (float) releaseTimeSecs;
}

//==============================================================================
bool ToyOrganSampledVoice::canPlaySound (juce::SynthesiserSound* s)
{
    return dynamic_cast<ToyOrganSampledSound*> (s) != nullptr;
}

void ToyOrganSampledVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* s, int currentPitchWheelPosition)
{
    if (auto* sound = dynamic_cast<ToyOrganSampledSound*> (s))
    {
        playingSound = sound;
        pitchRatio = std::pow (2.0, (midiNoteNumber - sound->getRootNote()) / 12.0)
                     * sound->getSourceSampleRate() / getSampleRate();
        sourceSamplePosition = 0.0;
        gain = velocity * 0.45f;

        adsr.setSampleRate (getSampleRate());
        adsr.setParameters (sound->getParams());
        adsr.noteOn();

        pitchWheelMoved (currentPitchWheelPosition);
        lfoPhase = 0.0;
    }
}

void ToyOrganSampledVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
        playingSound = nullptr;
    }
}

void ToyOrganSampledVoice::pitchWheelMoved (int newValue)
{
    pitchBendRatio = std::pow (2.0, (double) (newValue - 8192) / (8192.0 * 6.0));
}

void ToyOrganSampledVoice::controllerMoved (int controllerNumber, int newValue)
{
    if (controllerNumber == 1) // Modulation Wheel
        vibratoDepth = (float) newValue / 127.0f;
}

void ToyOrganSampledVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (playingSound == nullptr || playingSound->getLength() == 0)
        return;

    auto& soundData = playingSound->getData();
    const float* inL = soundData.getReadPointer (0);
    const float* inR = soundData.getNumChannels() > 1 ? soundData.getReadPointer (1) : nullptr;
    int soundLength = playingSound->getLength();

    float* outL = outputBuffer.getWritePointer (0, startSample);
    float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;

    const double lfoDelta = (2.0 * juce::MathConstants<double>::pi * 5.5) / getSampleRate();
    const double maxVibratoSemitones = 0.6;

    while (--numSamples >= 0)
    {
        auto pos = (int) sourceSamplePosition;
        if (pos >= soundLength - 1)
        {
            clearCurrentNote();
            playingSound = nullptr;
            break;
        }

        auto alpha = (float) (sourceSamplePosition - pos);
        auto invAlpha = 1.0f - alpha;

        float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

        auto env = adsr.getNextSample();
        if (! adsr.isActive())
        {
            clearCurrentNote();
            playingSound = nullptr;
            break;
        }

        l *= gain * env;
        r *= gain * env;

        if (outR != nullptr)
        {
            *outL++ += l;
            *outR++ += r;
        }
        else
        {
            *outL++ += (l + r) * 0.5f;
        }

        double vibratoRatio = 1.0;
        if (vibratoDepth > 0.001f)
        {
            double semitones = std::sin (lfoPhase) * (vibratoDepth * maxVibratoSemitones);
            vibratoRatio = std::pow (2.0, semitones / 12.0);
            lfoPhase += lfoDelta;
            if (lfoPhase >= 2.0 * juce::MathConstants<double>::pi)
                lfoPhase -= 2.0 * juce::MathConstants<double>::pi;
        }

        sourceSamplePosition += pitchRatio * pitchBendRatio * vibratoRatio;
    }
}

//==============================================================================
ToyOrganAudioProcessor::ToyOrganAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();

    // Register Master Volume parameter (DAW automatable, 0% to 100%, default 85%)
    addParameter (masterVolumeParam = new juce::AudioParameterFloat (
        juce::ParameterID ("volume", 1),
        "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.85f,
        juce::AudioParameterFloatAttributes().withLabel ("%").withStringFromValueFunction (
            [](float val, int) { return juce::String (juce::roundToInt (val * 100.0f)) + "%"; }
        )
    ));

    // 8 sampler voices + 8 synth fallback voices for polyphony
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new ToyOrganSampledVoice());
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new ToyOrganVoice());

    synth.addSound (new ToyOrganSound (*this));

    // Configure the 13 chromatic keys (C4 to C5: MIDI notes 60 to 72)
    const char* noteNames[13] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C" };
    for (int i = 0; i < 13; ++i)
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
    smoothVolume.reset (sampleRate, 0.02); // 20ms smoothing
    smoothVolume.setCurrentAndTargetValue (masterVolumeParam->get());
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

    // Filter incoming MIDI to strictly ONLY support the 13 keys (MIDI notes 60 to 72 = C4 to C5),
    // while preserving all pitch bend and CC controller messages!
    juce::MidiBuffer filteredMidi;
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOnOrOff())
        {
            int note = msg.getNoteNumber();
            if (note >= 60 && note <= 72)
                filteredMidi.addEvent (msg, metadata.samplePosition);
        }
        else
        {
            filteredMidi.addEvent (msg, metadata.samplePosition);
        }
    }
    midiMessages.clear();
    midiMessages.addEvents (filteredMidi, 0, buffer.getNumSamples(), 0);

    // Connect GUI / on-screen key presses to the audio MIDI stream
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    // Render sound (synth voices handle notes, pitch bend, and mod wheel)
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply smooth master volume
    smoothVolume.setTargetValue (masterVolumeParam->get());
    smoothVolume.applyGain (buffer, buffer.getNumSamples());
}

bool ToyOrganAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ToyOrganAudioProcessor::createEditor()
{
    return new ToyOrganAudioProcessorEditor (*this);
}

void ToyOrganAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("LaxersazOrganState");
    xml.setAttribute ("version", 1);
    xml.setAttribute ("soundsetName", currentSoundset.name);
    xml.setAttribute ("masterVolume", (double) masterVolumeParam->get());
    xml.setAttribute ("keyboardLayout", currentKeyboardLayout);
    xml.setAttribute ("editorWidth", lastEditorWidth);
    xml.setAttribute ("editorHeight", lastEditorHeight);

    copyXmlToBinary (xml, destData);
}

void ToyOrganAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName ("LaxersazOrganState"))
    {
        if (xml->hasAttribute ("masterVolume"))
            *masterVolumeParam = (float) xml->getDoubleAttribute ("masterVolume", 0.85);

        if (xml->hasAttribute ("keyboardLayout"))
            currentKeyboardLayout = xml->getIntAttribute ("keyboardLayout", 0);

        if (xml->hasAttribute ("editorWidth"))
            lastEditorWidth = xml->getIntAttribute ("editorWidth", 880);

        if (xml->hasAttribute ("editorHeight"))
            lastEditorHeight = xml->getIntAttribute ("editorHeight", 420);

        auto savedSoundsetName = xml->getStringAttribute ("soundsetName");
        if (savedSoundsetName.isNotEmpty())
        {
            for (int i = 0; i < (int) detectedSoundsetFolders.size(); ++i)
            {
                if (detectedSoundsetFolders[(size_t) i].getFileName() == savedSoundsetName)
                {
                    loadSoundsetByIndex (i);
                    break;
                }
            }
        }
    }
}

bool ToyOrganAudioProcessor::hasSampleForNote (int midiNoteNumber) const
{
    for (int i = 0; i < synth.getNumSounds(); ++i)
    {
        if (auto* samplerSound = dynamic_cast<ToyOrganSampledSound*> (synth.getSound (i).get()))
        {
            if (samplerSound->appliesToNote (midiNoteNumber))
                return true;
        }
    }
    return false;
}

bool ToyOrganAudioProcessor::isAnyKeyPressed() const
{
    for (int i = 0; i < 13; ++i)
    {
        if (keyboardState.isNoteOnForChannels (0xffff, keySlots[(size_t) i].noteNumber))
            return true;
    }
    return false;
}

int ToyOrganAudioProcessor::getCurrentlyPressedNote() const
{
    for (int i = 0; i < 13; ++i)
    {
        int note = keySlots[(size_t) i].noteNumber;
        if (keyboardState.isNoteOnForChannels (0xffff, note))
            return note;
    }
    return -1;
}

juce::File ToyOrganAudioProcessor::getSoundsetsFolder() const
{
    // 1. Current working directory (e.g. running from terminal or project root)
    auto cwdSoundsets = juce::File::getCurrentWorkingDirectory().getChildFile ("Soundsets");
    if (cwdSoundsets.isDirectory() && cwdSoundsets.getNumberOfChildFiles (juce::File::findDirectories) > 0)
        return cwdSoundsets;

    // 2. Search upwards from the current executable or app bundle (locates repo root when running from build/)
    auto dir = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (int i = 0; i < 8; ++i)
    {
        dir = dir.getParentDirectory();
        if (! dir.isDirectory() || dir.isRoot())
            break;

        auto candidate = dir.getChildFile ("Soundsets");
        if (candidate.isDirectory() && candidate.getNumberOfChildFiles (juce::File::findDirectories) > 0)
            return candidate;

        // macOS app bundle Resources directory
        auto bundleRes = dir.getChildFile ("Resources/Soundsets");
        if (bundleRes.isDirectory() && bundleRes.getNumberOfChildFiles (juce::File::findDirectories) > 0)
            return bundleRes;
    }

    // 3. User Music directory (default standard location for user soundsets and installed plugin)
    auto userMusicDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("LaxersazOrgan/Soundsets");
    if (! userMusicDir.exists())
    {
        auto legacyDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("ToyOrgan/Soundsets");
        if (legacyDir.isDirectory() && legacyDir.getNumberOfChildFiles (juce::File::findDirectories) > 0)
            return legacyDir;

        userMusicDir.createDirectory();
    }

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
    juce::String previousSoundsetName = currentSoundset.name;
    if (previousSoundsetName.isEmpty() && currentSoundsetIndex >= 0 && currentSoundsetIndex < (int) detectedSoundsetFolders.size())
        previousSoundsetName = detectedSoundsetFolders[(size_t) currentSoundsetIndex].getFileName();

    detectedSoundsetFolders.clear();
    soundsetNames.clear();

    auto primaryDir = getSoundsetsFolder();
    if (! primaryDir.exists())
        primaryDir.createDirectory();

    auto userMusicDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("LaxersazOrgan/Soundsets");
    if (! userMusicDir.exists())
    {
        auto legacyDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("ToyOrgan/Soundsets");
        if (legacyDir.isDirectory() && legacyDir.getNumberOfChildFiles (juce::File::findDirectories) > 0)
            userMusicDir = legacyDir;
    }

    // Automatically seed user music directory with factory soundsets if running from repo / bundle
    if (primaryDir != userMusicDir && primaryDir.isDirectory())
    {
        if (! userMusicDir.exists())
            userMusicDir.createDirectory();

        auto factorySets = primaryDir.findChildFiles (juce::File::findDirectories, false);
        for (const auto& fset : factorySets)
        {
            auto dest = userMusicDir.getChildFile (fset.getFileName());
            if (! dest.exists())
            {
                fset.copyDirectoryTo (dest);
            }
            else
            {
                auto childFiles = fset.findChildFiles (juce::File::findFiles, false);
                for (const auto& cf : childFiles)
                {
                    auto destFile = dest.getChildFile (cf.getFileName());
                    if (! destFile.existsAsFile() || cf.getFileName() == "color.txt")
                        cf.copyFileTo (destFile);
                }
            }
        }
    }

    // Scan primary soundset directory
    auto subdirs = primaryDir.findChildFiles (juce::File::findDirectories, false);
    for (auto& dir : subdirs)
    {
        detectedSoundsetFolders.push_back (dir);
        soundsetNames.add (dir.getFileName());
    }

    // Also scan userMusicDir if distinct from primaryDir to pick up any custom soundsets
    if (primaryDir != userMusicDir && userMusicDir.isDirectory())
    {
        auto userSubdirs = userMusicDir.findChildFiles (juce::File::findDirectories, false);
        for (auto& dir : userSubdirs)
        {
            if (! soundsetNames.contains (dir.getFileName()))
            {
                detectedSoundsetFolders.push_back (dir);
                soundsetNames.add (dir.getFileName());
            }
        }
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
        currentSoundset.keyColor = juce::Colour (0xFFFFEAA7);
    }
    else
    {
        int targetIndex = 0;
        if (previousSoundsetName.isNotEmpty())
        {
            for (int i = 0; i < (int) detectedSoundsetFolders.size(); ++i)
            {
                if (detectedSoundsetFolders[(size_t) i].getFileName().equalsIgnoreCase (previousSoundsetName))
                {
                    targetIndex = i;
                    break;
                }
            }
        }
        loadSoundsetByIndex (targetIndex);
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

static juce::Colour parseHexOrRgbColour (juce::String str, juce::Colour fallback)
{
    str = str.trim();
    if (str.isEmpty())
        return fallback;

    // Check tokens for hex color code
    for (const auto& token : juce::StringArray::fromTokens (str, " \t\r\n=:,;\"'{}", ""))
    {
        auto t = token.trim();
        if (t.startsWithIgnoreCase ("0x"))
            t = t.substring (2);
        while (t.startsWithChar ('#'))
            t = t.substring (1);

        if (t.length() == 6 && t.containsOnly ("0123456789abcdefABCDEF"))
        {
            auto rgb = (juce::uint32) t.getHexValue32();
            return juce::Colour (0xFF000000 | rgb);
        }
        if (t.length() == 8 && t.containsOnly ("0123456789abcdefABCDEF"))
        {
            return juce::Colour ((juce::uint32) t.getHexValue32());
        }
    }

    // Check for rgb(r, g, b) or r, g, b
    auto digitsAndCommas = str.retainCharacters ("0123456789,");
    auto commaTokens = juce::StringArray::fromTokens (digitsAndCommas, ",", "");
    if (commaTokens.size() >= 3)
    {
        int r = commaTokens[0].getIntValue();
        int g = commaTokens[1].getIntValue();
        int b = commaTokens[2].getIntValue();
        int a = (commaTokens.size() >= 4) ? commaTokens[3].getIntValue() : 255;
        if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
            return juce::Colour ((juce::uint8) r, (juce::uint8) g, (juce::uint8) b, (juce::uint8) a);
    }

    return fallback;
}

void ToyOrganAudioProcessor::loadSoundsetFromFolder (const juce::File& folder)
{
    currentSoundset.name = folder.getFileName();
    currentSoundset.folder = folder;
    currentSoundset.hasSamples = false;
    currentSoundset.sampleCount = 0;
    currentSoundset.mascotIdle = juce::Image();
    currentSoundset.mascotActive = juce::Image();
    currentSoundset.keyColor = juce::Colour (0xFFFFEAA7);

    // Look for soundset key background color definition
    const char* colorFiles[] = { "color.txt", "colors.txt", "background.txt", "key_color.txt", "soundset.json", "README.txt" };
    for (const char* cf : colorFiles)
    {
        auto file = folder.getChildFile (cf);
        if (file.existsAsFile())
        {
            auto content = file.loadFileAsString();
            juce::Colour parsed = parseHexOrRgbColour (content, juce::Colour());
            if (! parsed.isTransparent())
            {
                currentSoundset.keyColor = parsed;
                break;
            }
        }
    }

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

    // 2. Identify Idle and Active Image Sequences
    currentSoundset.idleFrames.clear();
    currentSoundset.activeFrames.clear();

    auto idleDir = folder.getChildFile ("idle");
    if (! idleDir.isDirectory())
        idleDir = folder.getChildFile ("idle_frames");

    auto activeDir = folder.getChildFile ("active");
    if (! activeDir.isDirectory())
        activeDir = folder.getChildFile ("active_frames");

    juce::Array<juce::File> idleFilesList;
    juce::Array<juce::File> activeFilesList;

    auto isImg = [](const juce::File& f)
    {
        auto ext = f.getFileExtension().toLowerCase();
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp";
    };

    if (idleDir.isDirectory())
    {
        for (const auto& f : idleDir.findChildFiles (juce::File::findFiles, false))
            if (isImg (f))
                idleFilesList.add (f);
    }

    if (activeDir.isDirectory())
    {
        for (const auto& f : activeDir.findChildFiles (juce::File::findFiles, false))
            if (isImg (f))
                activeFilesList.add (f);
    }

    // If subdirectories were not present, scan the main soundset folder
    if (idleFilesList.isEmpty() && activeFilesList.isEmpty())
    {
        for (const auto& img : imageFiles)
        {
            auto base = img.getFileNameWithoutExtension().toLowerCase();
            if (base.contains ("idle") || base.contains ("ruhe") || base.contains ("normal") || base.contains ("closed"))
                idleFilesList.add (img);
            else if (base.contains ("active") || base.contains ("aktiv") || base.contains ("gaak") || base.contains ("sing") || base.contains ("open"))
                activeFilesList.add (img);
        }
    }

    // Fallbacks if one or both categories were not explicitly identified
    if (idleFilesList.isEmpty() && imageFiles.size() > 0)
        idleFilesList.add (imageFiles[0]);

    if (activeFilesList.isEmpty())
    {
        if (imageFiles.size() > 1 && imageFiles[1] != idleFilesList[0])
            activeFilesList.add (imageFiles[1]);
        else if (idleFilesList.size() > 0)
            activeFilesList.add (idleFilesList[0]);
    }

    // Sort files naturally by filename (01, 02, 03... or a, b, c...)
    auto fileSorter = [](const juce::File& a, const juce::File& b)
    {
        return a.getFileName().toLowerCase() < b.getFileName().toLowerCase();
    };
    std::sort (idleFilesList.begin(), idleFilesList.end(), fileSorter);
    std::sort (activeFilesList.begin(), activeFilesList.end(), fileSorter);

    for (const auto& f : idleFilesList)
    {
        auto img = juce::ImageFileFormat::loadFrom (f);
        if (img.isValid())
            currentSoundset.idleFrames.push_back (img);
    }

    for (const auto& f : activeFilesList)
    {
        auto img = juce::ImageFileFormat::loadFrom (f);
        if (img.isValid())
            currentSoundset.activeFrames.push_back (img);
    }

    if (! currentSoundset.idleFrames.empty())
        currentSoundset.mascotIdle = currentSoundset.idleFrames[0];
    else
        currentSoundset.mascotIdle = juce::Image();

    if (! currentSoundset.activeFrames.empty())
        currentSoundset.mascotActive = currentSoundset.activeFrames[0];
    else
        currentSoundset.mascotActive = juce::Image();

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
            noteBits.setRange (60, 13, true);
            synth.addSound (new ToyOrganSampledSound (
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

        for (int i = 0; i < 13; ++i)
        {
            juce::File matchedFile;
            juce::String indexStr = juce::String (i + 1);
            juce::String indexPadded = (i + 1 < 10 ? "0" : "") + indexStr;

            // 1. Exact match on number (e.g. "01" or "1")
            for (const auto& af : audioFiles)
            {
                auto base = af.getFileNameWithoutExtension().toLowerCase().trim();
                if (base == indexPadded || base == indexStr)
                {
                    matchedFile = af;
                    break;
                }
            }

            // 2. Prefix match (e.g. "01_" or "01-")
            if (! matchedFile.existsAsFile())
            {
                for (const auto& af : audioFiles)
                {
                    auto base = af.getFileNameWithoutExtension().toLowerCase().trim();
                    if (base.startsWith (indexPadded + "_") || base.startsWith (indexPadded + "-")
                     || base.startsWith (indexStr + "_")    || base.startsWith (indexStr + "-"))
                    {
                        matchedFile = af;
                        break;
                    }
                }
            }

            // 3. Fallback: index in sorted files
            if (! matchedFile.existsAsFile() && i < audioFiles.size())
                matchedFile = audioFiles[i];

            if (matchedFile.existsAsFile())
            {
                std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (matchedFile));
                if (reader != nullptr)
                {
                    juce::BigInteger noteBits;
                    noteBits.setBit (60 + i);
                    synth.addSound (new ToyOrganSampledSound (
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

juce::String ToyOrganAudioProcessor::getShortcutForSlot (int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= 13)
        return {};

    // 0 = QWERTY
    static const char* qwertyShortcuts[13] = {
        "A", "W", "S", "E", "D", "F", "T", "G", "Y", "H", "U", "J", "K"
    };

    // 1 = QWERTZ (German / Austrian / Swiss: Z replaces Y)
    static const char* qwertzShortcuts[13] = {
        "A", "W", "S", "E", "D", "F", "T", "G", "Z", "H", "U", "J", "K"
    };

    // 2 = AZERTY (French / Belgian: Q replaces A, Z replaces W)
    static const char* azertyShortcuts[13] = {
        "Q", "Z", "S", "E", "D", "F", "T", "G", "Y", "H", "U", "J", "K"
    };

    if (currentKeyboardLayout == 1)
        return qwertzShortcuts[slotIndex];
    if (currentKeyboardLayout == 2)
        return azertyShortcuts[slotIndex];

    return qwertyShortcuts[slotIndex];
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToyOrganAudioProcessor();
}
