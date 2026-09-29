#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
// Original UI (D-018): no third-party trademarks, artwork or trade dress.
namespace refractor {
class RefractorProcessor;
class RefractorEditor : public juce::AudioProcessorEditor {
public:
  explicit RefractorEditor(RefractorProcessor&);
  ~RefractorEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override;
};
}
