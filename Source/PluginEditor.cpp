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

    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xFFF8FAFC));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00000000));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x00000000));
}

void ToyOrganLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused (minSliderPos, maxSliderPos, style, slider);
    float trackHeight = 6.0f;
    float trackY = (float) y + ((float) height - trackHeight) * 0.5f;
    auto trackBounds = juce::Rectangle<float> ((float) x, trackY, (float) width, trackHeight);

    // Dark track background
    g.setColour (juce::Colour (0xFF1E232B));
    g.fillRoundedRectangle (trackBounds, trackHeight * 0.5f);

    // Active fill
    float fillWidth = juce::jlimit (0.0f, (float) width, sliderPos - (float) x);
    if (fillWidth > 0.0f)
    {
        g.setColour (juce::Colour (0xFFFF7F00));
        g.fillRoundedRectangle (trackBounds.withWidth (fillWidth), trackHeight * 0.5f);
    }

    // Outer border
    g.setColour (juce::Colour (0xFF3F495A));
    g.drawRoundedRectangle (trackBounds, trackHeight * 0.5f, 1.0f);

    // Thumb handle
    float thumbRadius = 7.0f;
    auto thumbBounds = juce::Rectangle<float> (sliderPos - thumbRadius, (float) y + ((float) height * 0.5f) - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);
    g.setColour (juce::Colour (0x20000000));
    g.fillEllipse (thumbBounds.translated (0.0f, 1.5f));

    g.setColour (juce::Colours::white);
    g.fillEllipse (thumbBounds);

    g.setColour (juce::Colour (0xFFFF7F00));
    g.drawEllipse (thumbBounds, 2.0f);
}

void ToyOrganLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                       int buttonX, int buttonY, int buttonW, int buttonH,
                                       juce::ComboBox& box)
{
    juce::ignoreUnused (buttonX, buttonY, buttonW, buttonH, isButtonDown);
    auto bounds = juce::Rectangle<float> (1.0f, 1.0f, (float) width - 2.0f, (float) height - 2.0f);
    float corner = 6.0f;

    juce::Colour bg = box.findColour (juce::ComboBox::backgroundColourId);
    if (box.isMouseOver())
        bg = bg.brighter (0.05f);
    g.setColour (bg);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (box.isMouseOver() ? juce::Colour (0xFFFF7F00) : box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, corner, 1.2f);

    auto arrowArea = bounds.removeFromRight (18.0f);
    juce::Path arrow;
    float cx = arrowArea.getCentreX() - 2.0f;
    float cy = arrowArea.getCentreY();
    arrow.startNewSubPath (cx - 3.5f, cy - 1.5f);
    arrow.lineTo (cx, cy + 2.0f);
    arrow.lineTo (cx + 3.5f, cy - 1.5f);

    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.strokePath (arrow, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font ToyOrganLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    juce::ignoreUnused (box);
    return juce::FontOptions (12.0f, juce::Font::bold);
}

void ToyOrganLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (2, 1, box.getWidth() - 20, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void ToyOrganLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    float corner = 6.0f;

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
// ToyOrganIconButton Implementation (Crisp vector icons)
//==============================================================================
ToyOrganIconButton::ToyOrganIconButton (IconType type, const juce::String& tooltipText)
    : juce::Button (juce::String()), iconType (type)
{
    if (tooltipText.isNotEmpty())
        setTooltip (tooltipText);
}

void ToyOrganIconButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    float corner = 6.0f;

    juce::Colour bg (0xFF242A36);
    if (shouldDrawButtonAsDown)
        bg = bg.darker (0.15f);
    else if (shouldDrawButtonAsHighlighted)
        bg = bg.brighter (0.08f);

    g.setColour (juce::Colour (0x20000000));
    g.fillRoundedRectangle (bounds.translated (0.0f, 1.5f), corner);

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (shouldDrawButtonAsHighlighted ? juce::Colour (0xFFFF7F00) : juce::Colour (0xFF3F495A));
    g.drawRoundedRectangle (bounds, corner, 1.2f);

    auto iconColour = shouldDrawButtonAsHighlighted ? juce::Colour (0xFFFF7F00) : juce::Colour (0xFFF8FAFC);
    g.setColour (iconColour);

    auto center = bounds.getCentre();

    switch (iconType)
    {
        case IconType::Prev:
        {
            juce::Path p;
            p.startNewSubPath (center.x + 2.5f, center.y - 5.0f);
            p.lineTo (center.x - 2.5f, center.y);
            p.lineTo (center.x + 2.5f, center.y + 5.0f);
            g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        }
        case IconType::Next:
        {
            juce::Path p;
            p.startNewSubPath (center.x - 2.5f, center.y - 5.0f);
            p.lineTo (center.x + 2.5f, center.y);
            p.lineTo (center.x - 2.5f, center.y + 5.0f);
            g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        }
        case IconType::Folder:
        {
            float fw = 14.0f;
            float fh = 10.0f;
            float fx = center.x - fw * 0.5f;
            float fy = center.y - fh * 0.5f;

            juce::Path p;
            p.startNewSubPath (fx, fy + 2.0f);
            p.lineTo (fx + 4.5f, fy + 2.0f);
            p.lineTo (fx + 6.0f, fy + 3.5f);
            p.lineTo (fx + fw, fy + 3.5f);
            p.lineTo (fx + fw, fy + fh);
            p.lineTo (fx, fy + fh);
            p.closeSubPath();
            g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
            g.drawLine (fx + 1.0f, fy + 5.0f, fx + fw - 1.0f, fy + 5.0f, 1.2f);
            break;
        }
        case IconType::Reload:
        {
            float r = 5.0f;
            juce::Path p;
            p.addCentredArc (center.x, center.y, r, r, 0.0f, juce::degreesToRadians (55.0f), juce::degreesToRadians (315.0f), true);
            g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            float endAngle = juce::degreesToRadians (315.0f);
            float ax = center.x + r * std::sin (endAngle);
            float ay = center.y - r * std::cos (endAngle);

            juce::Path head;
            head.startNewSubPath (ax - 2.0f, ay - 3.5f);
            head.lineTo (ax + 1.0f, ay);
            head.lineTo (ax - 3.5f, ay + 1.0f);
            g.strokePath (head, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        }
    }
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

    // Note badge with keyboard shortcut hint (e.g. "C [A]", "C# [W]")
    auto shortcut = processor.getShortcutForSlot (slotIndex);
    juce::String badgeText = slot.keyName;
    if (shortcut.isNotEmpty())
        badgeText << " [" << shortcut << "]";

    float badgeW = isAccidental ? 42.0f : 46.0f;
    float badgeH = 18.0f;
    auto badgeRect = juce::Rectangle<float> (labelRect.getCentreX() - badgeW * 0.5f,
                                             labelRect.getCentreY() - badgeH * 0.5f,
                                             badgeW, badgeH);

    g.setColour (isPressed ? juce::Colour (0xFFFF7F00) : juce::Colour (0xF0FFFFFF));
    g.fillRoundedRectangle (badgeRect, 5.0f);

    g.setColour (isPressed ? juce::Colour (0xFFFF7F00) : juce::Colour (0x20000000));
    g.drawRoundedRectangle (badgeRect, 5.0f, 1.0f);

    g.setFont (juce::FontOptions (isAccidental ? 10.5f : 11.5f, juce::Font::bold));
    g.setColour (isPressed ? juce::Colours::white : juce::Colour (0xFF0D1926));
    g.drawText (badgeText, badgeRect, juce::Justification::centred, false);
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

    // 1. Header Left: Branding Title
    titleLabel.setText ("LAXERSAZ ORGAN", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFFFFFFF));
    addAndMakeVisible (titleLabel);

    // 2. Header Center: Soundset Preset Bar (< [Soundset Name] > [📁] [↻])
    prevSoundsetButton.onClick = [this]()
    {
        int count = audioProcessor.getSoundsetNames().size();
        if (count > 0)
        {
            int newIdx = (audioProcessor.getCurrentSoundsetIndex() - 1 + count) % count;
            audioProcessor.loadSoundsetByIndex (newIdx);
            populateSoundsets();
            for (auto& kv : keyViews)
                kv->repaint();
            repaint();
        }
    };
    addAndMakeVisible (prevSoundsetButton);

    addAndMakeVisible (soundsetComboBox);
    soundsetComboBox.setJustificationType (juce::Justification::centred);
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

    nextSoundsetButton.onClick = [this]()
    {
        int count = audioProcessor.getSoundsetNames().size();
        if (count > 0)
        {
            int newIdx = (audioProcessor.getCurrentSoundsetIndex() + 1) % count;
            audioProcessor.loadSoundsetByIndex (newIdx);
            populateSoundsets();
            for (auto& kv : keyViews)
                kv->repaint();
            repaint();
        }
    };
    addAndMakeVisible (nextSoundsetButton);

    openFolderButton.onClick = [this]()
    {
        audioProcessor.openSoundsetsFolder();
    };
    addAndMakeVisible (openFolderButton);

    refreshButton.onClick = [this]()
    {
        audioProcessor.scanForSoundsets();
        populateSoundsets();
        for (auto& kv : keyViews)
            kv->repaint();
        repaint();
    };
    addAndMakeVisible (refreshButton);

    // 3. Header Right: Keyboard layout selector & Master Volume
    layoutComboBox.addItem ("QWERTY", 1);
    layoutComboBox.addItem ("QWERTZ", 2);
    layoutComboBox.addItem ("AZERTY", 3);
    layoutComboBox.setSelectedId (audioProcessor.getKeyboardLayout() + 1, juce::dontSendNotification);
    layoutComboBox.onChange = [this]()
    {
        audioProcessor.setKeyboardLayout (layoutComboBox.getSelectedId() - 1);
        for (auto& kv : keyViews)
            kv->repaint();
    };
    addAndMakeVisible (layoutComboBox);

    volumeLabel.setText ("VOL", juce::dontSendNotification);
    volumeLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    volumeLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF94A3B8));
    volumeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (volumeLabel);

    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange (0.0, 1.0, 0.01);
    volumeSlider.setValue (audioProcessor.getMasterVolumeParam()->get(), juce::dontSendNotification);
    volumeSlider.onValueChange = [this]()
    {
        *audioProcessor.getMasterVolumeParam() = (float) volumeSlider.getValue();
        volValueLabel.setText (juce::String (juce::roundToInt (volumeSlider.getValue() * 100.0f)) + "%", juce::dontSendNotification);
    };
    addAndMakeVisible (volumeSlider);

    volValueLabel.setText (juce::String (juce::roundToInt (volumeSlider.getValue() * 100.0f)) + "%", juce::dontSendNotification);
    volValueLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    volValueLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFF8FAFC));
    volValueLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (volValueLabel);

    populateSoundsets();

    audioProcessor.getKeyboardState().addListener (this);

    // Enable keyboard focus for musical typing
    setWantsKeyboardFocus (true);

    // Resizable window support with persistent aspect ratio
    setResizable (true, true);
    setResizeLimits (660, 315, 1760, 840);
    getConstrainer()->setFixedAspectRatio (880.0 / 420.0);

    // Restore saved window size or default
    setSize (audioProcessor.getLastEditorWidth(), audioProcessor.getLastEditorHeight());

    // UI timer for 30Hz animation and state synchronization
    startTimerHz (30);
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
    auto topBarArea = juce::Rectangle<float> (0.0f, 0.0f, (float) getWidth(), 54.0f);
    g.setColour (juce::Colour (0xFF14171E));
    g.fillRect (topBarArea);

    // Divider line between top bar and lower area
    g.setColour (juce::Colour (0xFF262C38));
    g.drawHorizontalLine (54, 0.0f, (float) getWidth());

    // Logo at top left
    if (logoImage.isValid())
    {
        auto logoRect = juce::Rectangle<float> (16.0f, 9.0f, 36.0f, 36.0f);
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (logoImage, logoRect, juce::RectanglePlacement::centred);
    }

    // 2. Lower Area: Entire background below the top bar uses the soundset color
    auto lowerArea = juce::Rectangle<float> (0.0f, 55.0f, (float) getWidth(), (float) getHeight() - 55.0f);
    const auto& soundset = audioProcessor.getCurrentSoundset();
    g.setColour (soundset.keyColor);
    g.fillRect (lowerArea);
}

void ToyOrganAudioProcessorEditor::resized()
{
    audioProcessor.setLastEditorSize (getWidth(), getHeight());

    int w = getWidth();
    int h = getHeight();

    // -------------------------------------------------------------------------
    // 1. Header Left: Logo & Branding Title
    // -------------------------------------------------------------------------
    int titleW = (w < 720) ? 120 : 150;
    titleLabel.setBounds (56, 12, titleW, 30);
    int leftBoundary = titleLabel.getRight() + 8;

    // -------------------------------------------------------------------------
    // 2. Header Right: Master Volume & Keyboard Layout
    // -------------------------------------------------------------------------
    int rightEdge = w - 14;

    // Volume percentage label: 30px
    volValueLabel.setBounds (rightEdge - 30, 12, 30, 30);
    rightEdge = volValueLabel.getX() - 2;

    // Volume horizontal slider: responsive width between 46px and 65px
    int sliderW = juce::jlimit (46, 65, (w - 660) / 6 + 46);
    volumeSlider.setBounds (rightEdge - sliderW, 12, sliderW, 30);
    rightEdge = volumeSlider.getX() - 5;

    // "VOL" label: 24px
    volumeLabel.setBounds (rightEdge - 24, 12, 24, 30);
    rightEdge = volumeLabel.getX() - 10;

    // Keyboard layout selector dropdown: 76px to 86px
    int layoutW = (w < 720) ? 76 : 86;
    layoutComboBox.setBounds (rightEdge - layoutW, 12, layoutW, 30);
    int rightBoundary = layoutComboBox.getX() - 10;

    // -------------------------------------------------------------------------
    // 3. Header Center: Soundset Preset Bar (< [Name] > [📁] [↻])
    // -------------------------------------------------------------------------
    int btnW = 26;
    int btnGap = 3;
    int fixedBarW = btnW + btnGap + btnGap + btnW + 5 + btnW + btnGap + btnW; // 4 buttons + spacing = 118px

    int availableForBar = rightBoundary - leftBoundary;
    int maxComboW = availableForBar - fixedBarW - 12;
    int comboW = juce::jlimit (85, 135, maxComboW);

    int centerBarW = fixedBarW + comboW;
    int centerStartX = (w - centerBarW) / 2;

    // Anti-collision clamping: guarantee center bar NEVER overlaps left or right zones
    if (centerStartX < leftBoundary)
        centerStartX = leftBoundary;
    if (centerStartX + centerBarW > rightBoundary)
        centerStartX = rightBoundary - centerBarW;

    prevSoundsetButton.setBounds (centerStartX, 12, btnW, 30);
    soundsetComboBox.setBounds (prevSoundsetButton.getRight() + btnGap, 12, comboW, 30);
    nextSoundsetButton.setBounds (soundsetComboBox.getRight() + btnGap, 12, btnW, 30);
    openFolderButton.setBounds (nextSoundsetButton.getRight() + 5, 12, btnW, 30);
    refreshButton.setBounds (openFolderButton.getRight() + btnGap, 12, btnW, 30);

    // -------------------------------------------------------------------------
    // 4. Lower Area: Two Rows of Mascot Piano Keys
    // -------------------------------------------------------------------------
    auto lowerArea = juce::Rectangle<int> (0, 55, w, h - 55);

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

    // Sync volume slider if automated externally by DAW
    float currentVol = audioProcessor.getMasterVolumeParam()->get();
    if (! volumeSlider.isMouseButtonDown() && std::abs ((float) volumeSlider.getValue() - currentVol) > 0.005f)
    {
        volumeSlider.setValue (currentVol, juce::dontSendNotification);
        volValueLabel.setText (juce::String (juce::roundToInt (currentVol * 100.0f)) + "%", juce::dontSendNotification);
    }

    // Sync soundset dropdown if preset changed via state restore
    int curIdx = audioProcessor.getCurrentSoundsetIndex();
    if (soundsetComboBox.getSelectedId() != curIdx + 1)
        soundsetComboBox.setSelectedId (curIdx + 1, juce::dontSendNotification);

    // Sync layout combo if changed
    int curLayout = audioProcessor.getKeyboardLayout();
    if (layoutComboBox.getSelectedId() != curLayout + 1)
        layoutComboBox.setSelectedId (curLayout + 1, juce::dontSendNotification);

    // Check musical typing keys
    updateComputerKeyboardNotes();

    for (int i = 0; i < 13; ++i)
    {
        int note = audioProcessor.getKeySlots()[(size_t) i].noteNumber;
        bool isDown = audioProcessor.getKeyboardState().isNoteOnForChannels (0xffff, note);
        keyViews[(size_t) i]->setIsPressed (isDown);
        keyViews[(size_t) i]->updateAnimation (animTicks);
    }
}

bool ToyOrganAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    juce::ignoreUnused (key);
    updateComputerKeyboardNotes();
    return true;
}

bool ToyOrganAudioProcessorEditor::keyStateChanged (bool isKeyDown)
{
    juce::ignoreUnused (isKeyDown);
    updateComputerKeyboardNotes();
    return true;
}

void ToyOrganAudioProcessorEditor::focusLost (FocusChangeType)
{
    for (int i = 0; i < 13; ++i)
    {
        if (computerKeyHeld[i])
        {
            computerKeyHeld[i] = false;
            audioProcessor.getKeyboardState().noteOff (1, 60 + i, 0.0f);
            keyViews[(size_t) i]->setIsPressed (false);
        }
    }
}

void ToyOrganAudioProcessorEditor::updateComputerKeyboardNotes()
{
    if (! hasKeyboardFocus (true))
        return;

    int layout = audioProcessor.getKeyboardLayout();

    for (int i = 0; i < 13; ++i)
    {
        juce::String shortcut = audioProcessor.getShortcutForSlot (i);
        if (shortcut.isNotEmpty())
        {
            auto ch = shortcut[0];
            int keyCode = (int) juce::CharacterFunctions::toUpperCase (ch);
            bool isDown = juce::KeyPress::isKeyCurrentlyDown (keyCode);

            // Handle QWERTY / QWERTZ swap alias for G# (slot 8)
            if (i == 8)
            {
                if (layout == 1) // QWERTZ
                    isDown = isDown || juce::KeyPress::isKeyCurrentlyDown ('Y');
                else if (layout == 0) // QWERTY
                    isDown = isDown || juce::KeyPress::isKeyCurrentlyDown ('Z');
            }

            if (isDown != computerKeyHeld[i])
            {
                computerKeyHeld[i] = isDown;
                int midiNote = 60 + i;
                if (isDown)
                {
                    audioProcessor.getKeyboardState().noteOn (1, midiNote, 1.0f);
                    keyViews[(size_t) i]->setIsPressed (true);
                }
                else
                {
                    audioProcessor.getKeyboardState().noteOff (1, midiNote, 0.0f);
                    keyViews[(size_t) i]->setIsPressed (false);
                }
            }
        }
    }
}
