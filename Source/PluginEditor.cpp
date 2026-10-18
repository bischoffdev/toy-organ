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

    if (isAccidental)
    {
        // =====================================================================
        // BLACK KEY (Halbton) - Shorter, elevated "weiter oben"
        // =====================================================================
        // Soft drop shadow cast onto white keys below
        g.setColour (juce::Colour (0x38000000));
        g.fillRoundedRectangle (bounds.translated (0.0f, 4.0f).reduced (1.0f, 0.0f), 7.0f);

        auto keyRect = bounds.reduced (1.0f, 0.0f);
        keyRect.removeFromBottom (3.5f);

        // Body gradient: obsidian to deep graphite
        juce::Colour topColor = isPressed ? juce::Colour (0xFF475569) : juce::Colour (0xFF333842);
        juce::Colour botColor = isPressed ? juce::Colour (0xFF1E293B) : juce::Colour (0xFF111827);

        g.setGradientFill (juce::ColourGradient (topColor, keyRect.getCentreX(), keyRect.getY(),
                                                botColor, keyRect.getCentreX(), keyRect.getBottom(), false));

        juce::Path keyPath;
        keyPath.addRoundedRectangle (keyRect.getX(), keyRect.getY(), keyRect.getWidth(), keyRect.getHeight(),
                                     6.0f, 6.0f, false, false, true, true);
        g.fillPath (keyPath);

        // Key border / glow highlight
        g.setColour (isPressed ? juce::Colour (0xFFFF7675) : juce::Colour (0x28FFFFFF));
        g.strokePath (keyPath, juce::PathStrokeType (isPressed ? 2.0f : 1.0f));

        // Note label at bottom
        auto labelRect = juce::Rectangle<float> (keyRect.getX(), keyRect.getBottom() - 26.0f, keyRect.getWidth(), 22.0f);
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.setColour (isPressed ? juce::Colour (0xFFFFE66D) : juce::Colour (0xFFE2E8F0));
        g.drawText (slot.keyName, labelRect, juce::Justification::centred, false);

        // Mascot character medallion on the black key
        float badgeSize = juce::jmin (keyRect.getWidth() - 10.0f, 52.0f);
        float badgeY = labelRect.getY() - badgeSize - 6.0f;
        auto badgeRect = juce::Rectangle<float> (keyRect.getCentreX() - badgeSize * 0.5f,
                                                 badgeY + (isPressed ? 1.5f : 0.0f),
                                                 badgeSize, badgeSize);

        // Badge background
        g.setColour (isPressed ? juce::Colour (0xFF3B2727) : juce::Colour (0xFF1F2430));
        g.fillEllipse (badgeRect);
        g.setColour (isPressed ? juce::Colour (0xFFFF6B6B) : juce::Colour (0x44FFFFFF));
        g.drawEllipse (badgeRect, isPressed ? 1.8f : 1.0f);

        // Mascot image or vector fallback
        auto imgArea = badgeRect.reduced (3.0f);
        if (imgToDraw.isValid())
        {
            g.drawImage (imgToDraw, imgArea,
                         juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            drawVectorMascot (g, imgArea, isPressed);
        }
    }
    else
    {
        // =====================================================================
        // WHITE KEY (Ganzton) - Full height foundation
        // =====================================================================
        auto keyRect = bounds.reduced (1.2f, 0.0f);
        keyRect.removeFromBottom (3.0f);

        // Warm ivory vs pressed golden-yellow
        juce::Colour topColor = isPressed ? juce::Colour (0xFFFFFBEB) : juce::Colour (0xFFFFFFFF);
        juce::Colour botColor = isPressed ? juce::Colour (0xFFFFE69C) : juce::Colour (0xFFF1F5F9);

        g.setGradientFill (juce::ColourGradient (topColor, keyRect.getCentreX(), keyRect.getY(),
                                                botColor, keyRect.getCentreX(), keyRect.getBottom(), false));

        juce::Path keyPath;
        keyPath.addRoundedRectangle (keyRect.getX(), keyRect.getY(), keyRect.getWidth(), keyRect.getHeight(),
                                     8.0f, 8.0f, false, false, true, true);
        g.fillPath (keyPath);

        // Key border
        g.setColour (isPressed ? juce::Colour (0xFFF59E0B) : juce::Colour (0xFFCBD5E1));
        g.strokePath (keyPath, juce::PathStrokeType (isPressed ? 2.2f : 1.2f));

        // Note label at bottom of white key
        auto labelRect = juce::Rectangle<float> (keyRect.getX(), keyRect.getBottom() - 34.0f, keyRect.getWidth(), 30.0f);
        g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        g.setColour (isPressed ? juce::Colour (0xFFB45309) : juce::Colour (0xFF334155));
        g.drawText (slot.keyName, labelRect, juce::Justification::centred, false);

        // Mascot character medallion on the lower portion of the white key
        // (Positioned safely below where black keys hang down, completely unobstructed)
        float badgeSize = juce::jmin (keyRect.getWidth() - 16.0f, 96.0f);
        float badgeY = labelRect.getY() - badgeSize - 10.0f;
        auto badgeRect = juce::Rectangle<float> (keyRect.getCentreX() - badgeSize * 0.5f,
                                                 badgeY + (isPressed ? 2.0f : 0.0f),
                                                 badgeSize, badgeSize);

        // Circular frame badge
        g.setColour (isPressed ? juce::Colour (0xFFFFF7ED) : juce::Colour (0xFFF8FAFC));
        g.fillEllipse (badgeRect);
        g.setColour (isPressed ? juce::Colour (0xFFF97316) : juce::Colour (0xFFE2E8F0));
        g.drawEllipse (badgeRect, isPressed ? 2.4f : 1.4f);

        // Mascot image or vector fallback
        auto imgArea = badgeRect.reduced (4.0f);
        if (imgToDraw.isValid())
        {
            g.drawImage (imgToDraw, imgArea,
                         juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            drawVectorMascot (g, imgArea, isPressed);
        }
    }
}

//==============================================================================
ToyOrganAudioProcessorEditor::ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Create the 12 organ key visualizers (C4 to B4)
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

    // Title label
    titleLabel.setText (juce::CharPointer_UTF8 ("\xf0\x9f\x8e\xb9 LAXERSAZ ORGAN"), juce::dontSendNotification); // 🎹 LAXERSAZ ORGAN
    titleLabel.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF1E293B));
    addAndMakeVisible (titleLabel);

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
            updateStatus();
            for (auto& kv : keyViews)
                kv->repaint();
        }
    };

    // Status pill label in the header
    statusPillLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    statusPillLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF475569));
    statusPillLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusPillLabel);

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
        for (auto& kv : keyViews)
            kv->repaint();
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

void ToyOrganAudioProcessorEditor::updateStatus()
{
    const auto& soundset = audioProcessor.getCurrentSoundset();
    juce::String status = juce::String::fromUTF8 ("\xf0\x9f\x90\x94 ") + soundset.name; // 🐔
    if (soundset.hasSamples)
        status += " (" + juce::String (soundset.sampleCount) + " Samples)";
    else
        status += " (Orgel-Synth Modus)";

    statusPillLabel.setText (status, juce::dontSendNotification);
}

void ToyOrganAudioProcessorEditor::populateSoundsets()
{
    soundsetComboBox.clear (juce::dontSendNotification);
    auto names = audioProcessor.getSoundsetNames();

    for (int i = 0; i < names.size(); ++i)
        soundsetComboBox.addItem (names[i], i + 1);

    int currentIdx = audioProcessor.getCurrentSoundsetIndex();
    soundsetComboBox.setSelectedId (currentIdx + 1, juce::dontSendNotification);

    updateStatus();
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
    auto bounds = getLocalBounds().reduced (18);

    // Top Header: Title, Status, and Controls
    auto headerArea = bounds.removeFromTop (38);
    titleLabel.setBounds (headerArea.removeFromLeft (250));

    // Controls on the right of header
    refreshButton.setBounds (headerArea.removeFromRight (36));
    headerArea.removeFromRight (6);
    openFolderButton.setBounds (headerArea.removeFromRight (95));
    headerArea.removeFromRight (8);
    soundsetComboBox.setBounds (headerArea.removeFromRight (150));
    headerArea.removeFromRight (8);
    soundsetLabel.setBounds (headerArea.removeFromRight (70));
    headerArea.removeFromRight (10);

    // Status pill in the middle
    statusPillLabel.setBounds (headerArea);

    bounds.removeFromTop (12); // Spacing between header and keys

    // Keyboard Area occupies the remaining space
    auto keyboardBounds = bounds;

    // =========================================================================
    // PIANO KEYBOARD GEOMETRY: 7 White Keys + 5 Black Keys
    // =========================================================================
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

    // 5 Black keys are placed "weiter oben" with ~58% of key height
    struct BlackKeyConfig { int slotIndex; float crackPos; };
    const BlackKeyConfig blackConfigs[5] = {
        { 1, 1.0f }, // C# between C (0) and D (1)
        { 3, 2.0f }, // D# between D (1) and E (2)
        { 6, 4.0f }, // F# between F (3) and G (4)
        { 8, 5.0f }, // G# between G (4) and A (5)
        { 10, 6.0f } // A# between A (5) and B (6)
    };

    float blackW = whiteW * 0.60f;
    float blackH = keyH * 0.58f;

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
        keyViews[(size_t) index]->setIsPressed (true);
}

void ToyOrganAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState*, int, int midiNoteNumber, float)
{
    int index = midiNoteNumber - 60;
    if (index >= 0 && index < 12)
        keyViews[(size_t) index]->setIsPressed (false);
}

void ToyOrganAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 12; ++i)
    {
        int note = audioProcessor.getKeySlots()[(size_t) i].noteNumber;
        bool isDown = audioProcessor.getKeyboardState().isNoteOnForChannels (0xffff, note);
        keyViews[(size_t) i]->setIsPressed (isDown);
    }
}
