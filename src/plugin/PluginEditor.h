#pragma once
#include <array>
#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>
// Original UI (D-018): no third-party trademarks, artwork or trade dress.
namespace refractor {
class RefractorProcessor;
class RefractorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit RefractorEditor(RefractorProcessor&);
  ~RefractorEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override;
private:
  struct Foot;
  struct Knob {
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
  };
  void timerCallback() override;
  void addKnob(Knob& k, const char* key, const char* caption);
  std::array<Knob, 7> knobs_;
  juce::ToggleButton expButton_{"Expression"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> expAttachment_;
  std::unique_ptr<Foot> activeFoot_, magicFoot_;
};
}  // namespace refractor
