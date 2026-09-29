#include "plugin/PluginEditor.h"
#include "plugin/PluginProcessor.h"
#include "dsp/Errors.hpp"
namespace refractor {
RefractorEditor::RefractorEditor(RefractorProcessor& p) : juce::AudioProcessorEditor(p) { throw NotImplemented("RefractorEditor"); }
RefractorEditor::~RefractorEditor() = default;
void RefractorEditor::paint(juce::Graphics&) {}
void RefractorEditor::resized() {}
}
