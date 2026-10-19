#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToyOrganKeyView::ToyOrganKeyView (int slot, bool accidental, ToyOrganAudioProcessor& proc)
    : slotIndex (slot), isAccidental (accidental), processor (proc)
{
}

void ToyOrganKeyView::triggerNoteOn()
{
    setIsPressed (true);
    int note = processor.getKeySlots()[(size_t) slotIndex].noteNumber;
    processor.getKeyboardState().noteOn (1, note, 1.0f);
}

void ToyOrganKeyView::triggerNoteOff()
{
    setIsPressed (false);
    int note = processor.getKeySlots()[(size_t) slotIndex].noteNumber;
    processor.getKeyboardState().noteOff (1, note, 0.0f);
}

void ToyOrganKeyView::mouseDown (const juce::MouseEvent&)
{
    triggerNoteOn();
}

void ToyOrganKeyView::mouseUp (const juce::MouseEvent&)
{
    triggerNoteOff();
}

void ToyOrganKeyView::mouseEnter (const juce::MouseEvent& e)
{
    if (e.mods.isAnyMouseButtonDown())
        triggerNoteOn();
}

void ToyOrganKeyView::mouseExit (const juce::MouseEvent&)
{
    if (isPressed)
        triggerNoteOff();
}

void ToyOrganKeyView::setIsPressed (bool pressed)
{
    if (isPressed != pressed)
    {
        isPressed = pressed;
        repaint();
    }
}

void ToyOrganKeyView::drawVectorMascot (juce::Graphics& g, juce::Rectangle<float> area, bool pressed)
{
    auto center = area.getCentre();
    float radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.40f;

    // Head
    g.setColour (juce::Colour (0xFFFFD43B));
    g.fillEllipse (center.x - radius, center.y - radius * 0.9f, radius * 2.0f, radius * 1.9f);

    // Comb (Kamm) on top
    g.setColour (juce::Colour (0xFFFF4757));
    g.fillEllipse (center.x - radius * 0.35f, center.y - radius * 1.25f, radius * 0.7f, radius * 0.5f);

    // Eyes
    g.setColour (juce::Colour (0xFF2F3542));
    if (pressed)
    {
        // Happy singing eyes (> <)
        g.setFont (juce::FontOptions (radius * 0.65f, juce::Font::bold));
        g.drawText (">", juce::Rectangle<float> (center.x - radius * 0.55f, center.y - radius * 0.4f, radius * 0.5f, radius * 0.5f), juce::Justification::centred, false);
        g.drawText ("<", juce::Rectangle<float> (center.x + radius * 0.05f, center.y - radius * 0.4f, radius * 0.5f, radius * 0.5f), juce::Justification::centred, false);

        // Wide open beak (singing!)
        g.setColour (juce::Colour (0xFFFF7675));
        juce::Path beak;
        beak.startNewSubPath (center.x - radius * 0.45f, center.y + radius * 0.05f);
        beak.lineTo (center.x + radius * 0.45f, center.y + radius * 0.05f);
        beak.lineTo (center.x, center.y + radius * 0.65f);
        beak.closeSubPath();
        g.fillPath (beak);
    }
    else
    {
        // Calm resting eyes (• •)
        float eyeSize = radius * 0.22f;
        g.fillEllipse (center.x - radius * 0.45f, center.y - radius * 0.25f, eyeSize, eyeSize);
        g.fillEllipse (center.x + radius * 0.25f, center.y - radius * 0.25f, eyeSize, eyeSize);

        // Small closed beak
        g.setColour (juce::Colour (0xFFFA8231));
        juce::Path beak;
        beak.startNewSubPath (center.x - radius * 0.3f, center.y + radius * 0.1f);
        beak.lineTo (center.x + radius * 0.3f, center.y + radius * 0.1f);
        beak.lineTo (center.x, center.y + radius * 0.45f);
        beak.closeSubPath();
        g.fillPath (beak);
    }
}

void ToyOrganKeyView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto& slot = processor.getKeySlots()[(size_t) slotIndex];
    const auto& soundset = processor.getCurrentSoundset();

    juce::Image imgToDraw;
    if (isPressed && soundset.mascotActive.isValid())
        imgToDraw = soundset.mascotActive;
    else if (soundset.mascotIdle.isValid())
        imgToDraw = soundset.mascotIdle;

    // Both accidental (upper) and natural (lower) keys share the exact same background color
    juce::Colour baseColor = soundset.keyColor;

    // Do not change key color when pressed; the image change indicates the active key
    auto keyRect = bounds.reduced (1.0f);

    g.setColour (baseColor);

    juce::Path keyPath;
    keyPath.addRoundedRectangle (keyRect.getX(), keyRect.getY(), keyRect.getWidth(), keyRect.getHeight(),
                                 8.0f, 8.0f, false, false, true, true);
    g.fillPath (keyPath);

    // Outline all keys with the same white line
    g.setColour (juce::Colours::white);
    g.strokePath (keyPath, juce::PathStrokeType (1.5f));

    // Note label at bottom: #0d1926 font
    juce::Rectangle<float> labelRect;
    if (isAccidental)
        labelRect = juce::Rectangle<float> (keyRect.getX(), keyRect.getBottom() - 26.0f, keyRect.getWidth(), 22.0f);
    else
        labelRect = juce::Rectangle<float> (keyRect.getX(), keyRect.getBottom() - 32.0f, keyRect.getWidth(), 26.0f);

    g.setFont (juce::FontOptions (isAccidental ? 14.0f : 16.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xFF0D1926));
    g.drawText (slot.keyName, labelRect, juce::Justification::centred, false);

    // Mascot character on each key: drawn as wide as the key without circular frames
    float imgW = keyRect.getWidth() - 4.0f;
    float imgH = imgW;
    float imgY = 0.0f;

    if (isAccidental)
    {
        float availableH = labelRect.getY() - keyRect.getY() - 6.0f;
        imgH = juce::jmin (imgW, availableH);
        imgY = keyRect.getY() + 3.0f + (availableH - imgH) * 0.5f;
    }
    else
    {
        float blackH = bounds.getHeight() * 0.54f;
        float availableH = labelRect.getY() - blackH - 6.0f;
        imgH = juce::jmin (imgW, availableH);
        imgY = blackH + 3.0f + (availableH - imgH) * 0.5f;
    }

    auto imgArea = juce::Rectangle<float> (keyRect.getX() + 2.0f, imgY, imgW, imgH);

    if (imgToDraw.isValid())
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path clipPath;
        clipPath.addRoundedRectangle (imgArea, 6.0f);
        g.reduceClipRegion (clipPath);
        g.drawImage (imgToDraw, imgArea, juce::RectanglePlacement::centred);
    }
    else
    {
        drawVectorMascot (g, imgArea, isPressed);
    }
}

//==============================================================================
ToyOrganAudioProcessorEditor::ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Create the 13 organ key visualizers (C4 to C5: whole octave!)
    for (int i = 0; i < 13; ++i)
    {
        bool isAccidental = (i == 1 || i == 3 || i == 6 || i == 8 || i == 10);
        auto keyView = std::make_unique<ToyOrganKeyView> (i, isAccidental, audioProcessor);
        keyViews.push_back (std::move (keyView));
    }

    // Add white keys first (background) - 8 natural keys (C to high C)
    const int whiteSlotIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
    for (int idx : whiteSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // Add black keys second (foreground, higher Z-order) - 5 accidentals
    const int blackSlotIndices[5] = { 1, 3, 6, 8, 10 };
    for (int idx : blackSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // Title label: No keyboard icon, font #0d1926
    titleLabel.setText ("LAXERSAZ ORGAN", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF0D1926));
    addAndMakeVisible (titleLabel);

    // Soundset dropdown and toolbar
    soundsetLabel.setText ("Soundset:", juce::dontSendNotification);
    soundsetLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    soundsetLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF0D1926));
    addAndMakeVisible (soundsetLabel);

    soundsetComboBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFFFFFFFF));
    soundsetComboBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xFF0D1926));
    soundsetComboBox.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xFFFF7F00));
    soundsetComboBox.setColour (juce::ComboBox::arrowColourId, juce::Colour (0xFF0D1926));
    addAndMakeVisible (soundsetComboBox);
    soundsetComboBox.onChange = [this]()
    {
        int selectedId = soundsetComboBox.getSelectedId();
        if (selectedId > 0)
        {
            audioProcessor.loadSoundsetByIndex (selectedId - 1);
            for (auto& kv : keyViews)
                kv->repaint();
            repaint();
        }
    };

    openFolderButton.setButtonText (juce::CharPointer_UTF8 ("\xf0\x9f\x93\x81 Ordner...")); // 📁 Ordner...
    openFolderButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFFFF7ED));
    openFolderButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF0D1926));
    openFolderButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFF0D1926));
    openFolderButton.onClick = [this]()
    {
        audioProcessor.openSoundsetsFolder();
    };
    addAndMakeVisible (openFolderButton);

    refreshButton.setButtonText (juce::CharPointer_UTF8 ("\xf0\x9f\x94\x84")); // 🔄
    refreshButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFFFF7ED));
    refreshButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF0D1926));
    refreshButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFF0D1926));
    refreshButton.onClick = [this]()
    {
        audioProcessor.scanForSoundsets();
        populateSoundsets();
        for (auto& kv : keyViews)
            kv->repaint();
        repaint();
    };
    addAndMakeVisible (refreshButton);

    populateSoundsets();

    audioProcessor.getKeyboardState().addListener (this);

    // UI timer for 30Hz animation and state synchronization
    startTimerHz (30);

    setSize (840, 480);
}

ToyOrganAudioProcessorEditor::~ToyOrganAudioProcessorEditor()
{
    audioProcessor.getKeyboardState().removeListener (this);
    stopTimer();
}

void ToyOrganAudioProcessorEditor::populateSoundsets()
{
    soundsetComboBox.clear (juce::dontSendNotification);
    auto names = audioProcessor.getSoundsetNames();

    for (int i = 0; i < names.size(); ++i)
        soundsetComboBox.addItem (names[i], i + 1);

    int currentIdx = audioProcessor.getCurrentSoundsetIndex();
    soundsetComboBox.setSelectedId (currentIdx + 1, juce::dontSendNotification);
}

void ToyOrganAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Retro pastel casing background
    g.fillAll (juce::Colour (0xFFF8FAFC));

    // Toy casing container with border #ff7f00
    auto area = getLocalBounds().toFloat().reduced (12.0f);
    g.setColour (juce::Colour (0xFFFFFFFF));
    g.fillRoundedRectangle (area, 18.0f);

    g.setColour (juce::Colour (0xFFFF7F00)); // #ff7f00 border
    g.drawRoundedRectangle (area, 18.0f, 2.5f);
}

void ToyOrganAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (18);

    // Top Header: Title and Controls (headline removed)
    auto headerArea = bounds.removeFromTop (38);
    titleLabel.setBounds (headerArea.removeFromLeft (260));

    // Controls on the right of header
    refreshButton.setBounds (headerArea.removeFromRight (36));
    headerArea.removeFromRight (8);
    openFolderButton.setBounds (headerArea.removeFromRight (100));
    headerArea.removeFromRight (10);
    soundsetComboBox.setBounds (headerArea.removeFromRight (160));
    headerArea.removeFromRight (8);
    soundsetLabel.setBounds (headerArea.removeFromRight (70));

    bounds.removeFromTop (12); // Spacing between header and keys

    // Keyboard Area occupies the remaining space
    auto keyboardBounds = bounds;

    // 8 White keys (C4 to C5 whole octave)
    const int whiteSlotIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
    float totalW = (float) keyboardBounds.getWidth();
    float whiteW = totalW / 8.0f;
    float keyH = (float) keyboardBounds.getHeight();

    for (int i = 0; i < 8; ++i)
    {
        int slot = whiteSlotIndices[i];
        float x = (float) keyboardBounds.getX() + (float) i * whiteW;
        keyViews[(size_t) slot]->setBounds (juce::roundToInt (x),
                                            keyboardBounds.getY(),
                                            juce::roundToInt (whiteW),
                                            juce::roundToInt (keyH));
    }

    // 5 Black keys - as wide as the white keys!
    struct BlackKeyConfig { int slotIndex; float crackPos; };
    const BlackKeyConfig blackConfigs[5] = {
        { 1, 1.0f }, // C# between C (0) and D (1)
        { 3, 2.0f }, // D# between D (1) and E (2)
        { 6, 4.0f }, // F# between F (3) and G (4)
        { 8, 5.0f }, // G# between G (4) and A (5)
        { 10, 6.0f } // A# between A (5) and B (6)
    };

    float blackW = whiteW; // As wide as the white keys!
    float blackH = keyH * 0.54f;

    for (const auto& cfg : blackConfigs)
    {
        float crackX = (float) keyboardBounds.getX() + cfg.crackPos * whiteW;
        float x = crackX - (blackW * 0.5f);

        keyViews[(size_t) cfg.slotIndex]->setBounds (juce::roundToInt (x),
                                                     keyboardBounds.getY(),
                                                     juce::roundToInt (blackW),
                                                     juce::roundToInt (blackH));
        // Keep black keys in front so they receive clicks and render on top
        keyViews[(size_t) cfg.slotIndex]->toFront (false);
    }
}

void ToyOrganAudioProcessorEditor::handleNoteOn (juce::MidiKeyboardState*, int, int midiNoteNumber, float)
{
    int index = midiNoteNumber - 60;
    if (index >= 0 && index < 13)
        keyViews[(size_t) index]->setIsPressed (true);
}

void ToyOrganAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState*, int, int midiNoteNumber, float)
{
    int index = midiNoteNumber - 60;
    if (index >= 0 && index < 13)
        keyViews[(size_t) index]->setIsPressed (false);
}

void ToyOrganAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 13; ++i)
    {
        int note = audioProcessor.getKeySlots()[(size_t) i].noteNumber;
        bool isDown = audioProcessor.getKeyboardState().isNoteOnForChannels (0xffff, note);
        keyViews[(size_t) i]->setIsPressed (isDown);
    }
}
