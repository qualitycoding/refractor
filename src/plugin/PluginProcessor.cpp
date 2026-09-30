#include "plugin/PluginProcessor.h"
#include <cmath>
#include "plugin/PluginEditor.h"
#include "dsp/Params.hpp"

namespace refractor {
namespace {
constexpr const char* kStateTag = "REFRACTOR";   // D-016: same shape as an APVTS state tree (PARAM id/value)
constexpr int kParamVersion = 1;

// Accepts only plain decimal/exponent numerals; rejects nan/inf/hex/words and absurd lengths (D-016, R-005).
bool parseNumber(const juce::String& text, float& out) {
  const auto t = text.trim();
  if (t.isEmpty() || t.length() > 40 || !t.containsOnly("0123456789+-.eE")) return false;
  const double d = t.getDoubleValue();
  if (!std::isfinite(d)) return false;
  out = static_cast<float>(juce::jlimit(-1.0e30, 1.0e30, d));
  return true;
}
}  // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  for (const auto& s : parameterSpecs()) {
    const juce::ParameterID id{s.key, kParamVersion};
    if (s.isBool)
      layout.add(std::make_unique<juce::AudioParameterBool>(id, s.name, s.defaultValue >= 0.5f));
    else
      layout.add(std::make_unique<juce::AudioParameterFloat>(id, s.name, juce::NormalisableRange<float>(s.min, s.max), s.defaultValue));
  }
  return layout;
}

RefractorProcessor::RefractorProcessor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, kStateTag, createParameterLayout()) {
  for (const auto& s : parameterSpecs()) raw_[static_cast<std::size_t>(s.id)] = apvts_.getRawParameterValue(s.key);
}
RefractorProcessor::~RefractorProcessor() = default;

void RefractorProcessor::pushParameters() {
  for (const auto& s : parameterSpecs()) engine_.setParameter(s.id, raw_[static_cast<std::size_t>(s.id)]->load());
}
void RefractorProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  engine_.prepare(sampleRate, samplesPerBlock);
  pushParameters();
  engine_.reset();                      // snap smoothers to the current parameter values (D-015)
  setLatencySamples(0);                 // the wet lag is part of the effect
}
void RefractorProcessor::releaseResources() {}

bool RefractorProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {   // D-017
  const auto in = layouts.getMainInputChannelSet(), out = layouts.getMainOutputChannelSet();
  const bool inOk = in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
  const bool outOk = out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
  return inOk && outOk && !(in == juce::AudioChannelSet::stereo() && out == juce::AudioChannelSet::mono());
}

void RefractorProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  const int nIn = juce::jmin(getTotalNumInputChannels(), 2), nOut = juce::jmin(getTotalNumOutputChannels(), 2);
  for (int c = nOut; c < buffer.getNumChannels(); ++c) buffer.clear(c, 0, n);
  if (nIn < 1 || nOut < 1 || n <= 0) { buffer.clear(); return; }
  pushParameters();
  float* const* ch = buffer.getArrayOfWritePointers();
  engine_.process(ch, ch, nIn, nOut, n);
}

juce::AudioProcessorEditor* RefractorProcessor::createEditor() { return new RefractorEditor(*this); }
bool RefractorProcessor::hasEditor() const { return true; }
const juce::String RefractorProcessor::getName() const { return "Refractor"; }
bool RefractorProcessor::acceptsMidi() const { return false; }
bool RefractorProcessor::producesMidi() const { return false; }
bool RefractorProcessor::isMidiEffect() const { return false; }
double RefractorProcessor::getTailLengthSeconds() const { return Engine::kTailSeconds; }
int RefractorProcessor::getNumPrograms() { return 1; }
int RefractorProcessor::getCurrentProgram() { return 0; }
void RefractorProcessor::setCurrentProgram(int) {}
const juce::String RefractorProcessor::getProgramName(int) { return {}; }
void RefractorProcessor::changeProgramName(int, const juce::String&) {}

void RefractorProcessor::getStateInformation(juce::MemoryBlock& destData) {
  juce::XmlElement xml(kStateTag);
  for (const auto& s : parameterSpecs()) {
    auto* p = xml.createNewChildElement("PARAM");
    p->setAttribute("id", s.key);
    // Normalised value (== denormalised: every range is 0..1). The raw atomic of a bool parameter is snapped to 0/1 by JUCE,
    // which would lose the exact host-visible value across a save/restore.
    const float v = apvts_.getParameter(s.key) != nullptr ? apvts_.getParameter(s.key)->getValue() : s.defaultValue;
    p->setAttribute("value", juce::String(static_cast<double>(v), 9));
  }
  copyXmlToBinary(xml, destData);
}

// D-016: validated manual parse. Never throws, never trusts the blob: unknown ids ignored, non-numeric/non-finite values
// ignored, finite values clamped to the parameter range. Missing parameters keep their current value.
void RefractorProcessor::setStateInformation(const void* data, int sizeInBytes) {
  if (data == nullptr || sizeInBytes <= 0) return;
  const auto xml = getXmlFromBinary(data, sizeInBytes);
  if (xml == nullptr || !xml->hasTagName(kStateTag)) return;
  for (const auto* el : xml->getChildWithTagNameIterator("PARAM")) {
    const auto id = el->getStringAttribute("id");
    for (const auto& s : parameterSpecs()) {
      if (id != s.key) continue;
      float v = 0.0f;
      if (!parseNumber(el->getStringAttribute("value"), v)) break;
      v = juce::jlimit(s.min, s.max, v);
      // All ranges are 0..1, so the stored number is already normalised. (convertTo0to1 would snap bool parameters to 0/1.)
      if (auto* p = apvts_.getParameter(s.key)) p->setValueNotifyingHost(v);
      break;
    }
  }
}

juce::AudioProcessorParameter* RefractorProcessor::getBypassParameter() const {
  return apvts_.getParameter(spec(ParamId::Bypass).key);
}
juce::AudioProcessorValueTreeState& RefractorProcessor::state() { return apvts_; }
}  // namespace refractor

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new refractor::RefractorProcessor(); }
