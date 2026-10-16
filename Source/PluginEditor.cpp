#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ToyOrganKeyView::ToyOrganKeyView (int slot, ToyOrganAudioProcessor& proc)
    : slotIndex (slot), processor (proc)
{
}

void ToyOrganKeyView::setIsPressed (bool pressed)
{
    if (isPressed != pressed)
    {
        isPressed = pressed;
        repaint();
    }
}

void ToyOrganKeyView::mouseDown (const juce::MouseEvent&)
{
    setIsPressed (true);
    int note = processor.getKeySlots()[(size_t) slotIndex].noteNumber;
    processor.getKeyboardState().noteOn (1, note, 1.0f);
}

void ToyOrganKeyView::mouseUp (const juce::MouseEvent&)
{
    setIsPressed (false);
    int note = processor.getKeySlots()[(size_t) slotIndex].noteNumber;
    processor.getKeyboardState().noteOff (1, note, 0.0f);
}

void ToyOrganKeyView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (3.0f);
    const auto& slot = processor.getKeySlots()[(size_t) slotIndex];

    // Split bounds: top 55% for character/image, bottom 45% for organ key
    auto characterArea = bounds.removeFromTop (bounds.getHeight() * 0.58f);
    bounds.removeFromTop (4.0f); // gap
    auto keyArea = bounds;

    // 1. Draw character / sample state box
    auto charBgColor = isPressed ? juce::Colour (0xFFFFE66D) : juce::Colour (0xFFF7F7F7);
    g.setColour (charBgColor);
    g.fillRoundedRectangle (characterArea, 8.0f);

    g.setColour (isPressed ? juce::Colour (0xFFFF6B6B) : juce::Colour (0xFFD0D7DE));
    g.drawRoundedRectangle (characterArea, 8.0f, isPressed ? 2.5f : 1.5f);

    // Placeholder character graphic / text (Idle vs Active)
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.setColour (isPressed ? juce::Colour (0xFFFF4757) : juce::Colour (0xFF57606A));
    
    juce::String avatarFace = isPressed ? "( >w< )!" : "( ^-^ )";
    g.drawText (avatarFace, characterArea.reduced (2.0f), juce::Justification::centred, false);

    // 2. Draw Organ Key
    bool isAccidental = slot.keyName.containsChar ('#');
    auto keyBaseColor = isAccidental ? juce::Colour (0xFF2F3542) : juce::Colour (0xFFFFFFFF);
    if (isPressed)
        keyBaseColor = isAccidental ? juce::Colour (0xFF57606F) : juce::Colour (0xFFFFEAA7);

    g.setColour (keyBaseColor);
    g.fillRoundedRectangle (keyArea, 6.0f);

    g.setColour (juce::Colour (0xFF2F3542));
    g.drawRoundedRectangle (keyArea, 6.0f, 1.5f);

    // Key note label
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.setColour (isAccidental ? juce::Colours::white : juce::Colour (0xFF2F3542));
    g.drawText (slot.keyName, keyArea.removeFromBottom (26.0f), juce::Justification::centred, false);
}

//==============================================================================
ToyOrganAudioProcessorEditor::ToyOrganAudioProcessorEditor (ToyOrganAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Create the 12 organ key visualizers
    for (int i = 0; i < 12; ++i)
    {
        auto keyView = std::make_unique<ToyOrganKeyView> (i, audioProcessor);
        addAndMakeVisible (*keyView);
        keyViews.push_back (std::move (keyView));
    }

    // Title & header
    titleLabel.setText ("TOY ORGAN", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF2F3542));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("12-Key Customizable Sampler & Soundset Organ", juce::dontSendNotification);
    subtitleLabel.setFont (juce::FontOptions (13.0f));
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF747D8C));
    subtitleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitleLabel);

    audioProcessor.getKeyboardState().addListener (this);

    // Smooth UI timer for key status sync
    startTimerHz (30);

    setSize (780, 420);
}

ToyOrganAudioProcessorEditor::~ToyOrganAudioProcessorEditor()
{
    audioProcessor.getKeyboardState().removeListener (this);
    stopTimer();
}

//==============================================================================
void ToyOrganAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Retro pastel casing background
    g.fillAll (juce::Colour (0xFFF1F2F6));

    // Toy casing container
    auto area = getLocalBounds().toFloat().reduced (14.0f);
    g.setColour (juce::Colour (0xFFFAFAFC));
    g.fillRoundedRectangle (area, 16.0f);

    g.setColour (juce::Colour (0xFFDFE4EA));
    g.drawRoundedRectangle (area, 16.0f, 2.0f);
}

void ToyOrganAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);

    titleLabel.setBounds (bounds.removeFromTop (32));
    subtitleLabel.setBounds (bounds.removeFromTop (22));

    bounds.removeFromTop (16); // spacing

    // Layout the 12 keys evenly in one row
    auto keyWidth = bounds.getWidth() / 12;
    for (size_t i = 0; i < keyViews.size(); ++i)
    {
        keyViews[i]->setBounds (bounds.getX() + (int) i * keyWidth, 
                                bounds.getY(), 
                                keyWidth, 
                                bounds.getHeight());
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
