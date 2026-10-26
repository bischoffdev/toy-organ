#include "PluginProcessor.h"
#include "PluginEditor.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    ToyOrganAudioProcessor processor;
    if (argc > 1)
    {
        juce::String targetSet = argv[1];
        for (int i = 0; i < processor.getSoundsetNames().size(); ++i)
        {
            if (processor.getSoundsetNames()[i].equalsIgnoreCase (targetSet))
            {
                processor.loadSoundsetByIndex (i);
                break;
            }
        }
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());

    if (editor != nullptr)
    {
        editor->setSize (880, 420);

        juce::Image img (juce::Image::ARGB, 880, 420, true);
        juce::Graphics g (img);
        editor->paintEntireComponent (g, true);

        juce::String outputName = (argc > 2) ? argv[2] : "screenshot.png";
        juce::File outputFile = juce::File::getCurrentWorkingDirectory().getChildFile (outputName);
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
