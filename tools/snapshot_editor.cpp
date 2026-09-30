// Writes a PNG of the plugin editor at its default size. Usage (Linux needs a display): xvfb-run -a snapshot_editor <file.png>
#include <juce_audio_processors/juce_audio_processors.h>
#include "plugin/PluginProcessor.h"
int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI gui;
  refractor::RefractorProcessor proc;
  std::unique_ptr<juce::AudioProcessorEditor> editor(proc.createEditor());
  editor->setSize(640, 360);
  const auto image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 2.0f);
  juce::File out(argc > 1 ? argv[1] : "editor.png");
  out.getParentDirectory().createDirectory();
  out.deleteFile();
  juce::FileOutputStream stream(out);
  juce::PNGImageFormat png;
  return stream.openedOk() && png.writeImageToStream(image, stream) ? 0 : 1;
}
