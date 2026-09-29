// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <cstring>
#include <limits>
#include "dsp/Params.hpp"
#include "dsp/Mapping.hpp"
#include "dsp/Calibration.hpp"
using namespace refractor; using Catch::Matchers::WithinAbs; using Catch::Matchers::WithinRel;

// T-001 [unit] Parameter table is the frozen public interface. Enforces: SC-3, D-003.
TEST_CASE("T-001 parameter table matches D-003", "[unit][T-001]") {
  struct Row { const char* key; const char* name; float min, max, def; bool b; };
  const Row rows[] = {
    {"pitch","Pitch",0,1,0.5f,false}, {"pitch_exp","Pitch Exp",0,1,0.5f,false}, {"exp_enabled","Exp Enabled",0,1,0,true},
    {"primary","Primary",0,1,0.7f,false}, {"secondary","Secondary",0,1,0,false}, {"tracking","Tracking",0,1,0.8f,false},
    {"tone","Tone",0,1,1,false}, {"magic","Magic",0,1,0.3f,false}, {"magic_on","Magic On",0,1,0,true}, {"bypass","Bypass",0,1,0,true}};
  auto s = parameterSpecs();
  REQUIRE(s.size() == static_cast<size_t>(ParamId::Count));
  for (size_t i = 0; i < s.size(); ++i) {
    CHECK(static_cast<size_t>(s[i].id) == i);
    CHECK(std::strcmp(s[i].key, rows[i].key) == 0); CHECK(std::strcmp(s[i].name, rows[i].name) == 0);
    CHECK(s[i].min == rows[i].min); CHECK(s[i].max == rows[i].max); CHECK(s[i].defaultValue == rows[i].def);
    CHECK(s[i].isBool == rows[i].b);
    CHECK(&spec(s[i].id) == &s[i]);
  }
}

// T-002 [unit] Pitch knob -> semitones, piecewise linear, monotonic. Enforces: SC-2, C-003, D-004, A-013.
TEST_CASE("T-002 pitch mapping", "[unit][T-002]") {
  CHECK_THAT(mapping::pitchKnobToSemitones(0.0f), WithinAbs(calib::kPitchDownSemitones, 1e-6));
  CHECK_THAT(mapping::pitchKnobToSemitones(0.5f), WithinAbs(0.0, 1e-6));
  CHECK_THAT(mapping::pitchKnobToSemitones(1.0f), WithinAbs(calib::kPitchUpSemitones, 1e-6));
  CHECK_THAT(mapping::pitchKnobToSemitones(0.25f), WithinAbs(calib::kPitchDownSemitones * 0.5, 1e-5));
  CHECK_THAT(mapping::pitchKnobToSemitones(-3.0f), WithinAbs(calib::kPitchDownSemitones, 1e-6));   // clamp
  CHECK_THAT(mapping::pitchKnobToSemitones(7.0f), WithinAbs(calib::kPitchUpSemitones, 1e-6));
  CHECK_THAT(mapping::pitchKnobToSemitones(std::numeric_limits<float>::quiet_NaN()), WithinAbs(0.0, 1e-6)); // default 0.5
  float prev = -1e9f;
  for (int i = 0; i <= 1000; ++i) { float v = mapping::pitchKnobToSemitones(i / 1000.0f); CHECK(v >= prev); prev = v; }
  CHECK_THAT(mapping::semitonesToRatio(12.0f), WithinRel(2.0, 1e-6));
  CHECK_THAT(mapping::semitonesToRatio(-5.0f), WithinRel(std::pow(2.0, -5.0 / 12.0), 1e-6));
}

// T-003 [unit] Level law: unity at kUnityKnob ("2 o'clock"). Enforces: C-004, D-005, A-014.
TEST_CASE("T-003 level knob gain law", "[unit][T-003]") {
  CHECK(mapping::levelKnobToGain(0.0f) == 0.0f);
  CHECK_THAT(mapping::levelKnobToGain(calib::kUnityKnob), WithinAbs(1.0, 1e-6));
  CHECK_THAT(mapping::levelKnobToGain(1.0f), WithinAbs(1.0 / calib::kUnityKnob, 1e-6));
  CHECK_THAT(mapping::levelKnobToGain(2.0f), WithinAbs(1.0 / calib::kUnityKnob, 1e-6));
}

// T-004 [unit] Tracking -> internal clock scale, exponential, monotonic. Enforces: C-010, C-022, D-010.
TEST_CASE("T-004 tracking to clock scale", "[unit][T-004]") {
  CHECK_THAT(mapping::trackingToClockScale(0.0f), WithinRel(calib::kClockScaleMin, 1e-9));
  CHECK_THAT(mapping::trackingToClockScale(1.0f), WithinRel(calib::kClockScaleMax, 1e-9));
  CHECK_THAT(mapping::trackingToClockScale(0.5f), WithinRel(std::sqrt(calib::kClockScaleMin * calib::kClockScaleMax), 1e-6));
  double prev = 0; for (int i = 0; i <= 100; ++i) { double v = mapping::trackingToClockScale(i / 100.0f); CHECK(v > prev); prev = v; }
}

// T-005 [unit] Tone mapping: exponential 1 kHz..20 kHz, knob 1 = bypass (+inf). Enforces: C-005, D-007.
TEST_CASE("T-005 tone to cutoff", "[unit][T-005]") {
  CHECK_THAT(mapping::toneToCutoffHz(0.0f), WithinRel(calib::kToneMinCutoffHz, 1e-6));
  CHECK(std::isinf(mapping::toneToCutoffHz(1.0f)));
  CHECK_THAT(mapping::toneToCutoffHz(0.5f), WithinRel(std::sqrt(calib::kToneMinCutoffHz * 20000.0), 1e-4));
  CHECK_THAT(mapping::magicToFeedback(1.0f), WithinAbs(calib::kMagicMaxFeedback, 1e-6));
  CHECK(mapping::magicToFeedback(0.0f) == 0.0f);
}
