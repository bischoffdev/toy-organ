#include "PluginProcessor.h"
#include "PluginEditor.h"

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    ToyOrganAudioProcessor processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());

    if (editor != nullptr)
    {
        editor->setSize (880, 420);

        juce::Image img (juce::Image::ARGB, 880, 420, true);
        juce::Graphics g (img);
        editor->paintEntireComponent (g, true);

        juce::File outputFile = juce::File::getCurrentWorkingDirectory().getChildFile ("screenshot.png");
        outputFile.deleteFile();

        juce::FileOutputStream stream (outputFile);
        if (stream.openedOk())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream (img, stream);
        }
    }

    return 0;
}
