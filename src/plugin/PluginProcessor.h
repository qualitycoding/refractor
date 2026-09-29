#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/Engine.hpp"
// PUBLIC INTERFACE (D-003, D-015). Parameter IDs == refractor::ParamSpec::key. State = APVTS XML (D-016).
namespace refractor {
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

class RefractorProcessor : public juce::AudioProcessor {
public:
  RefractorProcessor();
  ~RefractorProcessor() override;
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;  // D-017
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;
  const juce::String getName() const override;
  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;          // Engine::kTailSeconds
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int) override;
  const juce::String getProgramName(int) override;
  void changeProgramName(int, const juce::String&) override;
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;  // must never crash on any input (T-032)
  juce::AudioProcessorParameter* getBypassParameter() const override;    // the "bypass" parameter
  juce::AudioProcessorValueTreeState& state();
private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RefractorProcessor)
};
}
