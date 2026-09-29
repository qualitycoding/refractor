// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstring>
#include <juce_audio_processors/juce_audio_processors.h>
#include "plugin/PluginProcessor.h"
#include "dsp/Params.hpp"
#include "support/TestSupport.hpp"
using namespace refractor;

namespace {
struct Init { juce::ScopedJuceInitialiser_GUI gui; };
juce::RangedAudioParameter* byKey(RefractorProcessor& p, const char* key) {
  for (auto* prm : p.getParameters()) if (auto* r = dynamic_cast<juce::RangedAudioParameter*>(prm)) if (r->getParameterID() == key) return r;
  return nullptr;
}
}

// T-030 [integration] Host-facing parameters mirror D-003 exactly. Enforces: SC-3, SC-4, D-003, D-015.
TEST_CASE("T-030 plugin parameters", "[plugin][T-030]") {
  Init init; RefractorProcessor p;
  CHECK(p.getParameters().size() == int(ParamId::Count));
  for (const auto& s : parameterSpecs()) {
    auto* r = byKey(p, s.key); REQUIRE(r != nullptr);
    CHECK(r->getNormalisableRange().start == s.min); CHECK(r->getNormalisableRange().end == s.max);
    CHECK(std::abs(r->convertFrom0to1(r->getDefaultValue()) - s.defaultValue) <= 1e-6f);
    CHECK(r->getName(64) == juce::String(s.name));
    CHECK(r->isBoolean() == s.isBool);
  }
  REQUIRE(p.getBypassParameter() != nullptr);
  CHECK(p.getBypassParameter() == byKey(p, "bypass"));
}

// T-031 [integration] State round-trip. Enforces: SC-3, D-016.
TEST_CASE("T-031 state round trip", "[plugin][T-031]") {
  Init init; RefractorProcessor a, b; uint32_t s = 9u;
  for (auto* prm : a.getParameters()) { s = s * 1664525u + 1013904223u; prm->setValueNotifyingHost(float(s >> 8) / 16777216.0f); }
  juce::MemoryBlock mb; a.getStateInformation(mb); b.setStateInformation(mb.getData(), int(mb.getSize()));
  for (int i = 0; i < a.getParameters().size(); ++i)
    CHECK(std::abs(a.getParameters()[i]->getValue() - b.getParameters()[i]->getValue()) <= 1e-6f);
}

// T-032 [security] setStateInformation never crashes on hostile input; params stay finite and in range. Enforces: SC-8, D-016, R-005.
TEST_CASE("T-032 state fuzzing", "[plugin][security][T-032]") {
  Init init; RefractorProcessor src; juce::MemoryBlock valid; src.getStateInformation(valid);
  const juce::String xml = juce::String::fromUTF8(static_cast<const char*>(valid.getData()), int(valid.getSize()));
  uint32_t s = 1234u; auto rnd = [&] { s = s * 1664525u + 1013904223u; return s >> 8; };
  for (int iter = 0; iter < 2000; ++iter) {
    RefractorProcessor p; juce::MemoryBlock blob;
    switch (iter % 5) {
      case 0: { blob.setSize(rnd() % 4096); for (size_t i = 0; i < blob.getSize(); ++i) static_cast<uint8_t*>(blob.getData())[i] = uint8_t(rnd()); break; }
      case 1: { blob = valid; blob.setSize(rnd() % (valid.getSize() + 1)); break; }
      case 2: { blob = valid; for (int k = 0; k < 16 && blob.getSize() > 0; ++k) static_cast<uint8_t*>(blob.getData())[rnd() % blob.getSize()] = uint8_t(rnd()); break; }
      case 3: { const char* vals[] = {"nan","inf","-inf","1e38","-1e38","","abc","0x10"};
                auto t = juce::String("<?xml version=\"1.0\"?><REFRACTOR>");
                for (const auto& sp : parameterSpecs()) t << "<PARAM id=\"" << sp.key << "\" value=\"" << vals[rnd() % 8] << "\"/>";
                t << "</REFRACTOR>"; blob.replaceAll(t.toRawUTF8(), t.getNumBytesAsUTF8()); break; }
      default: { auto t = xml.replace("value=\"", "value=\"-9e9"); blob.replaceAll(t.toRawUTF8(), t.getNumBytesAsUTF8()); break; }
    }
    REQUIRE_NOTHROW(p.setStateInformation(blob.getData(), int(blob.getSize())));
    for (auto* prm : p.getParameters()) { const float v = prm->getValue(); CHECK(std::isfinite(v)); CHECK(v >= 0.0f); CHECK(v <= 1.0f); }
  }
  RefractorProcessor p; REQUIRE_NOTHROW(p.setStateInformation(nullptr, 0));
}

// T-033 [integration] Latency 0 (lag is the effect), tail = Engine::kTailSeconds. Enforces: SC-7, D-015.
TEST_CASE("T-033 latency and tail", "[plugin][T-033]") {
  Init init; RefractorProcessor p; p.prepareToPlay(48000, 256);
  CHECK(p.getLatencySamples() == 0); CHECK(p.getTailLengthSeconds() == Engine::kTailSeconds);
}

// T-034 [integration] Supported bus layouts {1->1, 1->2, 2->2}. Enforces: SC-6, D-017.
TEST_CASE("T-034 bus layouts", "[plugin][T-034]") {
  Init init; RefractorProcessor p; using CS = juce::AudioChannelSet;
  auto L = [](CS i, CS o) { juce::AudioProcessor::BusesLayout l; l.inputBuses.add(i); l.outputBuses.add(o); return l; };
  CHECK(p.isBusesLayoutSupported(L(CS::mono(), CS::mono()))); CHECK(p.isBusesLayoutSupported(L(CS::mono(), CS::stereo())));
  CHECK(p.isBusesLayoutSupported(L(CS::stereo(), CS::stereo()))); CHECK_FALSE(p.isBusesLayoutSupported(L(CS::stereo(), CS::mono())));
  CHECK_FALSE(p.isBusesLayoutSupported(L(CS::create5point1(), CS::create5point1()))); CHECK_FALSE(p.isBusesLayoutSupported(L(CS::disabled(), CS::stereo())));
}

// T-035 [integration] processBlock equals Engine output bitwise for identical settings. Enforces: D-002, D-015.
TEST_CASE("T-035 processor wraps engine faithfully", "[plugin][T-035]") {
  Init init; RefractorProcessor p; const double fs = 48000;
  p.setPlayConfigDetails(2, 2, fs, 256); p.prepareToPlay(fs, 256);
  auto set = [&](const char* k, float v) { auto* r = byKey(p, k); r->setValueNotifyingHost(r->convertTo0to1(v)); };
  set("pitch", 0.8f); set("primary", 0.9f); set("secondary", 0.4f); set("tracking", 0.5f); set("magic", 0.5f); set("magic_on", 1.0f);
  p.releaseResources(); p.prepareToPlay(fs, 256);   // prepare snaps smoothers to current values (D-015)
  Engine e; e.prepare(fs, 256); rt::neutral(e);
  e.setParameter(ParamId::Pitch, 0.8f); e.setParameter(ParamId::Primary, 0.9f); e.setParameter(ParamId::Secondary, 0.4f);
  e.setParameter(ParamId::Tracking, 0.5f); e.setParameter(ParamId::Magic, 0.5f); e.setParameter(ParamId::MagicEngaged, 1.0f); e.reset();
  const auto x = rt::noise(size_t(fs), 17u); juce::MidiBuffer midi; juce::AudioBuffer<float> buf(2, 256);
  std::vector<float> ref0(256), ref1(256); float maxd = 0;
  for (size_t off = 0; off + 256 <= x.size(); off += 256) {
    for (int c = 0; c < 2; ++c) buf.copyFrom(c, 0, x.data() + off, 256);
    const float* in[2] = { x.data() + off, x.data() + off }; float* out[2] = { ref0.data(), ref1.data() };
    e.process(in, out, 2, 2, 256); p.processBlock(buf, midi);
    for (int n = 0; n < 256; ++n) maxd = std::max({ maxd, std::abs(buf.getSample(0, n) - ref0[size_t(n)]), std::abs(buf.getSample(1, n) - ref1[size_t(n)]) });
  }
  CHECK(maxd == 0.0f);
}
