#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>

//==============================================================================
// ToyOrganLookAndFeel Implementation (Light on Dark for Header Bar)
//==============================================================================
ToyOrganLookAndFeel::ToyOrganLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFF242A36));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xFFFF7F00));
    setColour (juce::ComboBox::textColourId, juce::Colour (0xFFF8FAFC));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (0xFFF8FAFC));

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xFF1E232B));
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xFFF8FAFC));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xFFFF7F00));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xFFFFFFFF));

    setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF242A36));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xFFF8FAFC));
    setColour (juce::TextButton::textColourOnId, juce::Colour (0xFFFFFFFF));
}

void ToyOrganLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                       int buttonX, int buttonY, int buttonW, int buttonH,
                                       juce::ComboBox& box)
{
    juce::ignoreUnused (buttonX, buttonY, buttonW, buttonH, isButtonDown);
    auto bounds = juce::Rectangle<float> (1.0f, 1.0f, (float) width - 2.0f, (float) height - 2.0f);
    float corner = bounds.getHeight() * 0.5f;

    juce::Colour bg = box.findColour (juce::ComboBox::backgroundColourId);
    if (box.isMouseOver())
        bg = bg.brighter (0.05f);
    g.setColour (bg);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, corner, 1.5f);

    auto arrowArea = bounds.removeFromRight (28.0f);
    juce::Path arrow;
    float cx = arrowArea.getCentreX();
    float cy = arrowArea.getCentreY();
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo (cx, cy + 2.5f);
    arrow.lineTo (cx + 4.0f, cy - 2.0f);

    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.strokePath (arrow, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void ToyOrganLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    float corner = bounds.getHeight() * 0.5f;

    juce::Colour base = backgroundColour;
    if (shouldDrawButtonAsDown)
        base = base.darker (0.15f);
    else if (shouldDrawButtonAsHighlighted)
        base = base.brighter (0.08f);

    g.setColour (juce::Colour (0x20000000));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.5f), corner);

    g.setColour (base);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (shouldDrawButtonAsHighlighted ? juce::Colour (0xFFFF7F00) : juce::Colour (0xFF3F495A));
    g.drawRoundedRectangle (bounds, corner, 1.2f);
}

//==============================================================================
// ToyOrganKeyView (Mascot Picture View)
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
        pressTicks = 0;
        repaint();
    }
}

void ToyOrganKeyView::updateAnimation (int animTick)
{
    globalAnimTick = animTick;
    if (isPressed)
        pressTicks++;
    else
        pressTicks = 0;

    const auto& soundset = processor.getCurrentSoundset();
    if ((isPressed && soundset.activeFrames.size() > 1) || (! isPressed && soundset.idleFrames.size() > 1))
        repaint();
}

bool ToyOrganKeyView::hitTest (int x, int y)
{
    return getLocalBounds().contains (x, y);
}

void ToyOrganKeyView::drawVectorMascot (juce::Graphics& g, juce::Rectangle<float> area, bool pressed)
{
    auto center = area.getCentre();
    float radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.42f;

    // Duck Head
    juce::ColourGradient headGrad (juce::Colour (0xFFFFE066), center.x, center.y - radius,
                                   juce::Colour (0xFFFFC021), center.x, center.y + radius, false);
    g.setGradientFill (headGrad);
    g.fillEllipse (center.x - radius, center.y - radius * 0.92f, radius * 2.0f, radius * 1.95f);

    // Duck Comb (Kamm) on top
    juce::ColourGradient combGrad (juce::Colour (0xFFFF6B6B), center.x, center.y - radius * 1.3f,
                                   juce::Colour (0xFFEE5253), center.x, center.y - radius * 0.8f, false);
    g.setGradientFill (combGrad);
    g.fillEllipse (center.x - radius * 0.35f, center.y - radius * 1.25f, radius * 0.70f, radius * 0.52f);

    // Cheek Blushes
    g.setColour (juce::Colour (0x35FF6B81));
    g.fillEllipse (center.x - radius * 0.75f, center.y + radius * 0.05f, radius * 0.38f, radius * 0.22f);
    g.fillEllipse (center.x + radius * 0.37f, center.y + radius * 0.05f, radius * 0.38f, radius * 0.22f);

    // Eyes & Beak
    g.setColour (juce::Colour (0xFF1E272E));
    if (pressed)
    {
        // Happy singing eyes (> <)
        g.setFont (juce::FontOptions (radius * 0.65f, juce::Font::bold));
        g.drawText (">", juce::Rectangle<float> (center.x - radius * 0.55f, center.y - radius * 0.42f, radius * 0.5f, radius * 0.5f), juce::Justification::centred, false);
        g.drawText ("<", juce::Rectangle<float> (center.x + radius * 0.05f, center.y - radius * 0.42f, radius * 0.5f, radius * 0.5f), juce::Justification::centred, false);

        // Wide open beak (singing!)
        g.setColour (juce::Colour (0xFFFF5252));
        juce::Path beak;
        beak.startNewSubPath (center.x - radius * 0.42f, center.y + radius * 0.08f);
        beak.lineTo (center.x + radius * 0.42f, center.y + radius * 0.08f);
        beak.lineTo (center.x, center.y + radius * 0.65f);
        beak.closeSubPath();
        g.fillPath (beak);

        // Pink singing tongue
        g.setColour (juce::Colour (0xFFFF7979));
        g.fillEllipse (center.x - radius * 0.18f, center.y + radius * 0.32f, radius * 0.36f, radius * 0.24f);
    }
    else
    {
        // Calm resting eyes (• •) with glossy catchlight
        float eyeSize = radius * 0.22f;
        g.fillEllipse (center.x - radius * 0.45f, center.y - radius * 0.25f, eyeSize, eyeSize);
        g.fillEllipse (center.x + radius * 0.23f, center.y - radius * 0.25f, eyeSize, eyeSize);

        g.setColour (juce::Colours::white);
        float spotSize = eyeSize * 0.35f;
        g.fillEllipse (center.x - radius * 0.45f + 1.0f, center.y - radius * 0.25f + 1.0f, spotSize, spotSize);
        g.fillEllipse (center.x + radius * 0.23f + 1.0f, center.y - radius * 0.25f + 1.0f, spotSize, spotSize);

        // Small cute beak
        g.setColour (juce::Colour (0xFFFA8231));
        juce::Path beak;
        beak.startNewSubPath (center.x - radius * 0.30f, center.y + radius * 0.10f);
        beak.lineTo (center.x + radius * 0.30f, center.y + radius * 0.10f);
        beak.lineTo (center.x, center.y + radius * 0.42f);
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
    if (isPressed)
        imgToDraw = soundset.getActiveFrame (pressTicks / 3);
    else
        imgToDraw = soundset.getIdleFrame ((globalAnimTick + slotIndex * 8) / 4);

    float labelH = 20.0f;
    float gap = 3.0f;
    float imgSize = bounds.getWidth() - 4.0f; // Square image

    juce::Rectangle<float> labelRect;
    juce::Rectangle<float> imgRect;

    if (isAccidental)
    {
        // Back row: Label ABOVE image
        labelRect = juce::Rectangle<float> (2.0f, 0.0f, bounds.getWidth() - 4.0f, labelH);
        imgRect   = juce::Rectangle<float> (2.0f, labelH + gap, imgSize, imgSize);
    }
    else
    {
        // Front row: Label BELOW image
        imgRect   = juce::Rectangle<float> (2.0f, 0.0f, imgSize, imgSize);
        labelRect = juce::Rectangle<float> (2.0f, imgSize + gap, bounds.getWidth() - 4.0f, labelH);
    }

    // Soft drop shadow under the image
    g.setColour (juce::Colour (0x25000000));
    g.fillRoundedRectangle (imgRect.translated (0.0f, 3.0f), 14.0f);

    // Draw the image (opaque, spanning the image rect)
    {
        juce::Graphics::ScopedSaveState state (g);
        juce::Path clipPath;
        clipPath.addRoundedRectangle (imgRect, 14.0f);
        g.reduceClipRegion (clipPath);

        // Opaque backing prevents background or overlap bleed-through
        g.setColour (juce::Colours::white);
        g.setOpacity (1.0f);
        g.fillRoundedRectangle (imgRect, 14.0f);

        if (imgToDraw.isValid())
        {
            g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            g.drawImage (imgToDraw, imgRect, juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);
        }
        else
        {
            g.setColour (juce::Colour (0xFFE2E8F0));
            g.fillRoundedRectangle (imgRect, 14.0f);
            drawVectorMascot (g, imgRect, isPressed);
        }
    }

    // Border around the image (orange when pressed, subtle white/semi-transparent when idle)
    g.setOpacity (1.0f);
    g.setColour (isPressed ? juce::Colour (0xFFFF7F00) : juce::Colour (0x60FFFFFF));
    g.drawRoundedRectangle (imgRect, 14.0f, isPressed ? 2.5f : 1.5f);

    // Note badge (above for back row, below for front row)
    float badgeW = isAccidental ? 30.0f : 34.0f;
    float badgeH = 18.0f;
    auto badgeRect = juce::Rectangle<float> (labelRect.getCentreX() - badgeW * 0.5f,
                                             labelRect.getCentreY() - badgeH * 0.5f,
                                             badgeW, badgeH);

    g.setColour (isPressed ? juce::Colour (0xFFFF7F00) : juce::Colour (0xF0FFFFFF));
    g.fillRoundedRectangle (badgeRect, 5.0f);

    g.setColour (isPressed ? juce::Colour (0xFFFF7F00) : juce::Colour (0x20000000));
    g.drawRoundedRectangle (badgeRect, 5.0f, 1.0f);

    g.setFont (juce::FontOptions (isAccidental ? 11.5f : 12.5f, juce::Font::bold));
    g.setColour (isPressed ? juce::Colours::white : juce::Colour (0xFF0D1926));
    g.drawText (slot.keyName, badgeRect, juce::Justification::centred, false);
}

//==============================================================================
// ToyOrganAudioProcessorEditor Implementation
//==============================================================================
ToyOrganAudioProcessorEditor::ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&customLookAndFeel);

    // Load logo image from embedded binary data
    logoImage = juce::ImageFileFormat::loadFrom (BinaryData::icon_128_png, BinaryData::icon_128_pngSize);

    // Create the 13 visual mascot picture views
    for (int i = 0; i < 13; ++i)
    {
        bool isAccidental = (i == 1 || i == 3 || i == 6 || i == 8 || i == 10);
        auto keyView = std::make_unique<ToyOrganKeyView> (i, isAccidental, audioProcessor);
        keyViews.push_back (std::move (keyView));
    }

    // 1. Add back row pictures first (background)
    const int blackSlotIndices[5] = { 1, 3, 6, 8, 10 };
    for (int idx : blackSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // 2. Add front row pictures second (foreground, higher Z-order)
    const int whiteSlotIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
    for (int idx : whiteSlotIndices)
        addAndMakeVisible (*keyViews[(size_t) idx]);

    // Title label in top bar (light text on dark bar)
    titleLabel.setText ("LAXERSAZ ORGAN", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFFFFFFF));
    addAndMakeVisible (titleLabel);

    // Soundset dropdown and toolbar in top bar
    soundsetLabel.setText ("Soundset:", juce::dontSendNotification);
    soundsetLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    soundsetLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFE2E8F0));
    soundsetLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (soundsetLabel);

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
    openFolderButton.onClick = [this]()
    {
        audioProcessor.openSoundsetsFolder();
    };
    addAndMakeVisible (openFolderButton);

    refreshButton.setButtonText (juce::CharPointer_UTF8 ("\xf0\x9f\x94\x84")); // 🔄
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

    setSize (880, 420);
}

ToyOrganAudioProcessorEditor::~ToyOrganAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
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
    // 1. Top Bar: Always light-on-dark, unaffected by soundset background color
    auto topBarArea = juce::Rectangle<float> (0.0f, 0.0f, (float) getWidth(), 50.0f);
    g.setColour (juce::Colour (0xFF14171E));
    g.fillRect (topBarArea);

    // Divider line between top bar and lower area
    g.setColour (juce::Colour (0xFF262C38));
    g.drawHorizontalLine (50, 0.0f, (float) getWidth());

    // Logo at top left
    if (logoImage.isValid())
    {
        auto logoRect = juce::Rectangle<float> (14.0f, 8.0f, 34.0f, 34.0f);
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (logoImage, logoRect, juce::RectanglePlacement::centred);
    }

    // 2. Lower Area: Entire background below the top bar uses the soundset color
    auto lowerArea = juce::Rectangle<float> (0.0f, 51.0f, (float) getWidth(), (float) getHeight() - 51.0f);
    const auto& soundset = audioProcessor.getCurrentSoundset();
    g.setColour (soundset.keyColor);
    g.fillRect (lowerArea);
}

void ToyOrganAudioProcessorEditor::resized()
{
    // Top Bar Area: 50px tall
    // Logo is drawn in paint at (14, 8) with size 34x34
    titleLabel.setBounds (54, 10, 200, 30);

    // Controls on the right of the top bar
    refreshButton.setBounds (getWidth() - 14 - 36, 8, 36, 34);
    openFolderButton.setBounds (refreshButton.getX() - 8 - 100, 8, 100, 34);
    soundsetComboBox.setBounds (openFolderButton.getX() - 10 - 160, 8, 160, 34);
    soundsetLabel.setBounds (soundsetComboBox.getX() - 8 - 70, 8, 70, 34);

    // -------------------------------------------------------------------------
    // Two Rows of Pictures (Layout matching piano keys with overlap)
    // -------------------------------------------------------------------------
    auto lowerArea = juce::Rectangle<int> (0, 51, getWidth(), getHeight() - 51);

    // 8 Natural notes: C4 (0), D4 (2), E4 (4), F4 (5), G4 (7), A4 (9), B4 (11), C5 (12)
    const int whiteSlotIndices[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };

    // 5 Accidental notes: C#4 (1), D#4 (3), F#4 (6), G#4 (8), A#4 (10)
    struct BlackKeyConfig { int slotIndex; float crackPos; };
    const BlackKeyConfig blackConfigs[5] = {
        { 1, 1.0f }, // C# between C (0) and D (1)
        { 3, 2.0f }, // D# between D (1) and E (2)
        { 6, 4.0f }, // F# between F (3) and G (4)
        { 8, 5.0f }, // G# between G (4) and A (5)
        { 10, 6.0f } // A# between A (5) and B (6)
    };

    float stageMarginX = 20.0f;
    float stageW = (float) getWidth() - (stageMarginX * 2.0f); // 840px
    float colWidth = stageW / 8.0f; // 105.0px per column

    // Back row images are AS LARGE AS front row images!
    float imgSize = colWidth * 0.96f; // ~101px square image
    float labelH = 20.0f;
    float gap = 3.0f;
    float componentW = imgSize;
    float componentH = imgSize + labelH + gap; // ~124px total component height

    float overlap = 12.0f; // Smaller overlap between front and back row
    float totalVerticalSpan = componentH + componentH - overlap; // ~218px

    // Center vertically in lower area
    float backY = (float) lowerArea.getY() + ((float) lowerArea.getHeight() - totalVerticalSpan) * 0.45f;
    float frontY = backY + componentH - overlap;

    for (const auto& cfg : blackConfigs)
    {
        float cx = stageMarginX + cfg.crackPos * colWidth;
        float x = cx - (componentW * 0.5f);
        keyViews[(size_t) cfg.slotIndex]->setBounds (juce::roundToInt (x),
                                                     juce::roundToInt (backY),
                                                     juce::roundToInt (componentW),
                                                     juce::roundToInt (componentH));
    }

    for (int i = 0; i < 8; ++i)
    {
        int slot = whiteSlotIndices[i];
        float cx = stageMarginX + ((float) i + 0.5f) * colWidth;
        float x = cx - (componentW * 0.5f);
        keyViews[(size_t) slot]->setBounds (juce::roundToInt (x),
                                            juce::roundToInt (frontY),
                                            juce::roundToInt (componentW),
                                            juce::roundToInt (componentH));
        // Keep front row in front so it overlaps the back row
        keyViews[(size_t) slot]->toFront (false);
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
    animTicks++;

    for (int i = 0; i < 13; ++i)
    {
        int note = audioProcessor.getKeySlots()[(size_t) i].noteNumber;
        bool isDown = audioProcessor.getKeyboardState().isNoteOnForChannels (0xffff, note);
        keyViews[(size_t) i]->setIsPressed (isDown);
        keyViews[(size_t) i]->updateAnimation (animTicks);
    }
}
