#include "plugin/PluginProcessor.h"
#include "plugin/PluginEditor.h"
#include "dsp/Errors.hpp"
namespace refractor {
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() { throw NotImplemented("createParameterLayout"); }
RefractorProcessor::RefractorProcessor() { throw NotImplemented("RefractorProcessor::RefractorProcessor"); }
RefractorProcessor::~RefractorProcessor() = default;
void RefractorProcessor::prepareToPlay(double, int) { throw NotImplemented("prepareToPlay"); }
void RefractorProcessor::releaseResources() {}
bool RefractorProcessor::isBusesLayoutSupported(const BusesLayout&) const { throw NotImplemented("isBusesLayoutSupported"); }
void RefractorProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) { throw NotImplemented("processBlock"); }
juce::AudioProcessorEditor* RefractorProcessor::createEditor() { return new RefractorEditor(*this); }
bool RefractorProcessor::hasEditor() const { return true; }
const juce::String RefractorProcessor::getName() const { return "Refractor"; }
bool RefractorProcessor::acceptsMidi() const { return false; }
bool RefractorProcessor::producesMidi() const { return false; }
bool RefractorProcessor::isMidiEffect() const { return false; }
double RefractorProcessor::getTailLengthSeconds() const { throw NotImplemented("getTailLengthSeconds"); }
int RefractorProcessor::getNumPrograms() { return 1; }
int RefractorProcessor::getCurrentProgram() { return 0; }
void RefractorProcessor::setCurrentProgram(int) {}
const juce::String RefractorProcessor::getProgramName(int) { return {}; }
void RefractorProcessor::changeProgramName(int, const juce::String&) {}
void RefractorProcessor::getStateInformation(juce::MemoryBlock&) { throw NotImplemented("getStateInformation"); }
void RefractorProcessor::setStateInformation(const void*, int) { throw NotImplemented("setStateInformation"); }
juce::AudioProcessorParameter* RefractorProcessor::getBypassParameter() const { throw NotImplemented("getBypassParameter"); }
juce::AudioProcessorValueTreeState& RefractorProcessor::state() { throw NotImplemented("state"); }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new refractor::RefractorProcessor(); }
