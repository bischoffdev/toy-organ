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

void ToyOrganKeyView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto& slot = processor.getKeySlots()[(size_t) slotIndex];

    if (isAccidental)
    {
        // =====================================================================
        // BLACK KEY (Halbton) - Arranged "weiter oben"
        // =====================================================================
        // Soft drop shadow cast onto white keys below
        g.setColour (juce::Colour (0x3A000000));
        g.fillRoundedRectangle (bounds.translated (0.0f, 3.5f).reduced (1.0f, 0.0f), 6.0f);

        auto keyRect = bounds.reduced (1.0f, 0.0f);
        keyRect.removeFromBottom (3.0f);

        // Body gradient: obsidian to deep graphite
        juce::Colour topColor = isPressed ? juce::Colour (0xFF4B5563) : juce::Colour (0xFF333842);
        juce::Colour botColor = isPressed ? juce::Colour (0xFF1F2937) : juce::Colour (0xFF14171E);

        g.setGradientFill (juce::ColourGradient (topColor, keyRect.getCentreX(), keyRect.getY(),
                                                botColor, keyRect.getCentreX(), keyRect.getBottom(), false));

        // Rounded bottom edge (like real piano sharps)
        juce::Path keyPath;
        keyPath.addRoundedRectangle (keyRect.getX(), keyRect.getY(), keyRect.getWidth(), keyRect.getHeight(),
                                     5.0f, 5.0f, false, false, true, true);
        g.fillPath (keyPath);

        // Subtle 3D top-left bevel highlight
        g.setColour (isPressed ? juce::Colour (0x66FF6B6B) : juce::Colour (0x22FFFFFF));
        g.strokePath (keyPath, juce::PathStrokeType (isPressed ? 1.8f : 1.0f));

        // Note label at the bottom of the black key
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.setColour (isPressed ? juce::Colour (0xFFFFE66D) : juce::Colour (0xFFE2E8F0));
        auto labelRect = keyRect.removeFromBottom (26.0f);
        g.drawText (slot.keyName, labelRect, juce::Justification::centred, false);
    }
    else
    {
        // =====================================================================
        // WHITE KEY (Ganzton) - Full height foundation
        // =====================================================================
        auto keyRect = bounds.reduced (1.0f, 0.0f);
        keyRect.removeFromBottom (2.0f);

        // Warm ivory vs joyful pressed yellow
        juce::Colour topColor = isPressed ? juce::Colour (0xFFFFFBEB) : juce::Colour (0xFFFFFFFF);
        juce::Colour botColor = isPressed ? juce::Colour (0xFFFFE69C) : juce::Colour (0xFFF1F3F5);

        g.setGradientFill (juce::ColourGradient (topColor, keyRect.getCentreX(), keyRect.getY(),
                                                botColor, keyRect.getCentreX(), keyRect.getBottom(), false));

        juce::Path keyPath;
        keyPath.addRoundedRectangle (keyRect.getX(), keyRect.getY(), keyRect.getWidth(), keyRect.getHeight(),
                                     7.0f, 7.0f, false, false, true, true);
        g.fillPath (keyPath);

        // Key border / separation crack
        g.setColour (isPressed ? juce::Colour (0xFFF59E0B) : juce::Colour (0xFFD0D7DE));
        g.strokePath (keyPath, juce::PathStrokeType (isPressed ? 1.8f : 1.0f));

        // Note label at the bottom of the white key
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.setColour (isPressed ? juce::Colour (0xFFB45309) : juce::Colour (0xFF334155));
        auto labelRect = keyRect.removeFromBottom (32.0f);
        g.drawText (slot.keyName, labelRect, juce::Justification::centred, false);
    }
}

//==============================================================================
ToyOrganMascotView::ToyOrganMascotView (ToyOrganAudioProcessor& proc)
    : processor (proc)
{
}

void ToyOrganMascotView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    const auto& soundset = processor.getCurrentSoundset();
    bool anyPressed = processor.isAnyKeyPressed();

    // 1. Retro Screen Bezel Frame
    g.setColour (juce::Colour (0x15000000));
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.5f), 14.0f);

    juce::Colour bgTop = anyPressed ? juce::Colour (0xFFFFF9DB) : juce::Colour (0xFFFAFAFD);
    juce::Colour bgBot = anyPressed ? juce::Colour (0xFFFFEEAA) : juce::Colour (0xFFEEF2F7);

    g.setGradientFill (juce::ColourGradient (bgTop, bounds.getCentreX(), bounds.getY(),
                                            bgBot, bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 14.0f);

    g.setColour (anyPressed ? juce::Colour (0xFFFF6B6B) : juce::Colour (0xFFCBD5E1));
    g.drawRoundedRectangle (bounds, 14.0f, anyPressed ? 2.2f : 1.5f);

    // 2. Soundset Status Pill Badge at bottom
    auto bottomBadge = bounds.removeFromBottom (26.0f).reduced (10.0f, 2.0f);
    g.setColour (juce::Colour (0x20000000));
    g.fillRoundedRectangle (bottomBadge, 6.0f);

    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xFF475569));
    juce::String statusText = "🐔 " + soundset.name;
    if (soundset.hasSamples)
        statusText += " (" + juce::String (soundset.sampleCount) + " Samples geladen)";
    else
        statusText += " (Orgel-Synth Modus)";
    g.drawText (statusText, bottomBadge, juce::Justification::centred, false);

    // 3. Display Idle / Active Character Picture
    auto characterArea = bounds.reduced (8.0f);

    juce::Image imageToDraw;
    if (anyPressed && soundset.mascotActive.isValid())
        imageToDraw = soundset.mascotActive;
    else if (soundset.mascotIdle.isValid())
        imageToDraw = soundset.mascotIdle;

    if (imageToDraw.isValid())
    {
        g.drawImage (imageToDraw,
                     characterArea,
                     juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    else
    {
        // Fallback Vector Cartoon Mascot (Chicken illustration)
        auto center = characterArea.getCentre();
        float radius = juce::jmin (characterArea.getWidth(), characterArea.getHeight()) * 0.38f;

        // Chicken head
        g.setColour (juce::Colour (0xFFFFD43B));
        g.fillEllipse (center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);

        // Chicken comb (Kamm) on top
        g.setColour (juce::Colour (0xFFFF4757));
        g.fillEllipse (center.x - 12.0f, center.y - radius - 10.0f, 24.0f, 16.0f);

        // Eyes
        g.setColour (juce::Colour (0xFF2F3542));
        if (anyPressed)
        {
            // Happy singing eyes (> <)
            g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
            g.drawText (">", juce::Rectangle<float> (center.x - radius * 0.5f, center.y - radius * 0.3f, 16.0f, 16.0f), juce::Justification::centred, false);
            g.drawText ("<", juce::Rectangle<float> (center.x + radius * 0.1f, center.y - radius * 0.3f, 16.0f, 16.0f), juce::Justification::centred, false);

            // Wide open beak (GAAK!)
            g.setColour (juce::Colour (0xFFFF7675));
            juce::Path beak;
            beak.startNewSubPath (center.x - 14.0f, center.y + 2.0f);
            beak.lineTo (center.x + 14.0f, center.y + 2.0f);
            beak.lineTo (center.x, center.y + 18.0f);
            beak.closeSubPath();
            g.fillPath (beak);
        }
        else
        {
            // Calm resting eyes (• •)
            g.fillEllipse (center.x - radius * 0.4f, center.y - radius * 0.2f, 6.0f, 6.0f);
            g.fillEllipse (center.x + radius * 0.2f, center.y - radius * 0.2f, 6.0f, 6.0f);

            // Small closed beak
            g.setColour (juce::Colour (0xFFFA8231));
            juce::Path beak;
            beak.startNewSubPath (center.x - 9.0f, center.y + 4.0f);
            beak.lineTo (center.x + 9.0f, center.y + 4.0f);
            beak.lineTo (center.x, center.y + 14.0f);
            beak.closeSubPath();
            g.fillPath (beak);
        }
    }

    // 4. Speech bubble or note indicator when active
    if (anyPressed)
    {
        auto bubbleArea = juce::Rectangle<float> (bounds.getRight() - 95.0f, bounds.getY() + 8.0f, 85.0f, 32.0f);
        g.setColour (juce::Colour (0xFFFF4757));
        g.fillRoundedRectangle (bubbleArea, 12.0f);

        int pressedNote = processor.getCurrentlyPressedNote();
        juce::String noteTxt = "GAAK! 🎵";
        if (pressedNote >= 60 && pressedNote < 72)
            noteTxt = processor.getKeySlots()[(size_t) (pressedNote - 60)].keyName + " 🎵";

        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.setColour (juce::Colours::white);
        g.drawText (noteTxt, bubbleArea, juce::Justification::centred, false);
    }
}

//==============================================================================
ToyOrganAudioProcessorEditor::ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), mascotView (p)
{
    // Create the 12 organ key visualizers
    // 0: C, 1: C#, 2: D, 3: D#, 4: E, 5: F, 6: F#, 7: G, 8: G#, 9: A, 10: A#, 11: B
    for (int i = 0; i < 12; ++i)
    {
        bool isAccidental = (i == 1 || i == 3 || i == 6 || i == 8 || i == 10);
        auto keyView = std::make_unique<ToyOrganKeyView> (i, isAccidental, audioProcessor);
        keyViews.push_back (std::move (keyView));
    }

    // Add white keys first (background)
    const int whiteSlotIndices[7] = { 0, 2, 4, 5, 7, 9, 11 };
    for (int idx : whiteSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // Add black keys second (foreground, higher Z-order)
    const int blackSlotIndices[5] = { 1, 3, 6, 8, 10 };
    for (int idx : blackSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // Mascot Display
    addAndMakeVisible (mascotView);

    // Title & header labels
    titleLabel.setText ("🎹 TOY ORGAN", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF1E293B));
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("12-Key Customizable Sampler & Soundset Organ", juce::dontSendNotification);
    subtitleLabel.setFont (juce::FontOptions (12.0f));
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF64748B));
    addAndMakeVisible (subtitleLabel);

    // Soundset dropdown and toolbar
    soundsetLabel.setText ("Soundset:", juce::dontSendNotification);
    soundsetLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    soundsetLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF334155));
    addAndMakeVisible (soundsetLabel);

    addAndMakeVisible (soundsetComboBox);
    soundsetComboBox.onChange = [this]()
    {
        int selectedId = soundsetComboBox.getSelectedId();
        if (selectedId > 0)
        {
            audioProcessor.loadSoundsetByIndex (selectedId - 1);
            mascotView.repaint();
        }
    };

    openFolderButton.setButtonText (juce::CharPointer_UTF8 ("\xf0\x9f\x93\x81 Ordner...")); // 📁 Ordner...
    openFolderButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFE2E8F0));
    openFolderButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF1E293B));
    openFolderButton.onClick = [this]()
    {
        audioProcessor.openSoundsetsFolder();
    };
    addAndMakeVisible (openFolderButton);

    refreshButton.setButtonText (juce::CharPointer_UTF8 ("\xf0\x9f\x94\x84")); // 🔄
    refreshButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFE2E8F0));
    refreshButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF1E293B));
    refreshButton.onClick = [this]()
    {
        audioProcessor.scanForSoundsets();
        populateSoundsets();
        mascotView.repaint();
    };
    addAndMakeVisible (refreshButton);

    populateSoundsets();

    audioProcessor.getKeyboardState().addListener (this);

    // UI timer for 30Hz animation and state synchronization
    startTimerHz (30);

    setSize (840, 520);
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
    g.fillAll (juce::Colour (0xFFF1F5F9));

    // Toy casing container with soft inner shadow
    auto area = getLocalBounds().toFloat().reduced (12.0f);
    g.setColour (juce::Colour (0xFFFFFFFF));
    g.fillRoundedRectangle (area, 18.0f);

    g.setColour (juce::Colour (0xFFCBD5E1));
    g.drawRoundedRectangle (area, 18.0f, 2.0f);
}

void ToyOrganAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);

    // Top Header: Title & Soundset Selector Bar
    auto headerArea = bounds.removeFromTop (38);
    auto titleArea = headerArea.removeFromLeft (320);
    titleLabel.setBounds (titleArea.removeFromTop (22));
    subtitleLabel.setBounds (titleArea);

    // Controls on the right of header
    refreshButton.setBounds (headerArea.removeFromRight (38));
    headerArea.removeFromRight (6);
    openFolderButton.setBounds (headerArea.removeFromRight (95));
    headerArea.removeFromRight (8);
    soundsetComboBox.setBounds (headerArea.removeFromRight (160));
    headerArea.removeFromRight (8);
    soundsetLabel.setBounds (headerArea.removeFromRight (70));

    bounds.removeFromTop (10); // spacing

    // Keyboard Area at the bottom (Height = 220px)
    auto keyboardBounds = bounds.removeFromBottom (220);

    // Mascot Stage in the center area above the keyboard
    auto mascotArea = bounds.reduced (12.0f, 4.0f);
    int mascotWidth = juce::jmin (360, mascotArea.getWidth());
    mascotView.setBounds (mascotArea.getCentreX() - mascotWidth / 2,
                          mascotArea.getY(),
                          mascotWidth,
                          mascotArea.getHeight());

    // =========================================================================
    // PIANO KEYBOARD GEOMETRY: Halbtöne (Black Keys) weiter oben
    // =========================================================================
    // 7 White keys span the full keyboard width and height:
    // C(0), D(2), E(4), F(5), G(7), A(9), B(11)
    const int whiteSlotIndices[7] = { 0, 2, 4, 5, 7, 9, 11 };
    float totalW = (float) keyboardBounds.getWidth();
    float whiteW = totalW / 7.0f;
    float keyH = (float) keyboardBounds.getHeight();

    for (int i = 0; i < 7; ++i)
    {
        int slot = whiteSlotIndices[i];
        float x = (float) keyboardBounds.getX() + (float) i * whiteW;
        keyViews[(size_t) slot]->setBounds (juce::roundToInt (x),
                                            keyboardBounds.getY(),
                                            juce::roundToInt (whiteW),
                                            juce::roundToInt (keyH));
    }

    // 5 Black keys (Halbtöne) are shorter (~60% height) and placed "weiter oben" (y = keyboardBounds.getY())
    // Centered at the division lines between white keys:
    // C# (slot 1)  -> between white 0 (C) and 1 (D) -> crack at 1.0 * whiteW
    // D# (slot 3)  -> between white 1 (D) and 2 (E) -> crack at 2.0 * whiteW
    // [E/F crack at 3.0 has NO black key]
    // F# (slot 6)  -> between white 3 (F) and 4 (G) -> crack at 4.0 * whiteW
    // G# (slot 8)  -> between white 4 (G) and 5 (A) -> crack at 5.0 * whiteW
    // A# (slot 10) -> between white 5 (A) and 6 (B) -> crack at 6.0 * whiteW
    struct BlackKeyConfig { int slotIndex; float crackPos; };
    const BlackKeyConfig blackConfigs[5] = {
        { 1, 1.0f },
        { 3, 2.0f },
        { 6, 4.0f },
        { 8, 5.0f },
        { 10, 6.0f }
    };

    float blackW = whiteW * 0.60f;
    float blackH = keyH * 0.60f;

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
    if (index >= 0 && index < 12)
    {
        keyViews[(size_t) index]->setIsPressed (true);
        mascotView.repaint();
    }
}

void ToyOrganAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState*, int, int midiNoteNumber, float)
{
    int index = midiNoteNumber - 60;
    if (index >= 0 && index < 12)
    {
        keyViews[(size_t) index]->setIsPressed (false);
        mascotView.repaint();
    }
}

void ToyOrganAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 12; ++i)
    {
        int note = audioProcessor.getKeySlots()[(size_t) i].noteNumber;
        bool isDown = audioProcessor.getKeyboardState().isNoteOnForChannels (0xffff, note);
        keyViews[(size_t) i]->setIsPressed (isDown);
    }
    mascotView.repaint();
}
