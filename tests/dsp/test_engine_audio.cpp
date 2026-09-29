// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
// Audio-behaviour tests of refractor::Engine. Tolerances are justified from spike S1
// (research/spikes/S1-delayline-shifter/output.json): max observed ratio error 0.61% within the pedal
// range (-5..+4 st) and 1.82% at octaves; tolerances add ~40-60% margin for resampling and chorus LFO.
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include "support/TestSupport.hpp"
#include "dsp/Mapping.hpp"
using namespace refractor; using namespace rt;

static constexpr double kRatioTolPedal = 0.010;  // 1.0 %  (~17 cents)
static constexpr double kRatioTolOct   = 0.025;  // 2.5 %

static Vec wetFor(double fs, float pitchKnob, float primary, float secondary, const Vec& in, float tracking = 1.0f) {
  Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, pitchKnob); e.setParameter(ParamId::Primary, primary);
  e.setParameter(ParamId::Secondary, secondary); e.setParameter(ParamId::Tracking, tracking); e.reset();
  return wetOf(run(e, in), in);
}

// T-006 [unit/audio] Primary voice pitch ratio across the knob. Enforces: SC-2, C-003, C-020, D-004, D-010.
TEST_CASE("T-006 primary pitch ratio accuracy", "[audio][T-006]") {
  const double fs = 48000; const Vec in = sine(440, fs, 1.5);
  for (float k : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) {
    const Vec w = wetFor(fs, k, 1.0f, 0.0f, in);
    const double f = dominantHz(w, fs, size_t(0.5 * fs));
    INFO("knob " << k << " measured " << f);
    CHECK(std::abs(f / (440.0 * ratioOf(expectedSemitones(k))) - 1.0) <= kRatioTolPedal);
  }
}

// T-007 [unit/audio] Dry path exact; nothing added when wet is off. Enforces: SC-5, D-002, D-014.
TEST_CASE("T-007 dry identity with wet off", "[audio][T-007]") {
  for (double fs : {44100.0, 48000.0, 96000.0}) {
    Engine e; e.prepare(fs, 512); neutral(e); e.reset();
    const Vec in = noise(size_t(fs), 7u);
    CHECK(maxAbsDiff(run(e, in), in) <= 1e-6f);
  }
}

// T-008 [unit/audio] Wet lag follows Tracking via the internal clock. Enforces: SC-2, C-009, C-010, C-022, D-010.
TEST_CASE("T-008 wet lag versus tracking", "[audio][T-008]") {
  const double fs = 48000; double prevLag = -1.0;
  for (float t : {1.0f, 0.75f, 0.5f, 0.25f, 0.0f}) {
    Engine e; e.prepare(fs, 512); neutral(e);
    e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Tracking, t); e.reset();
    const double fInt = calib::kNominalClockHz * mapping::trackingToClockScale(t);
    const double expected = (calib::kWindowSamples / 2.0) / fInt;
    CHECK(std::abs(e.nominalWetLagSeconds() - expected) <= 1e-9);
    Vec in(size_t(0.5 * fs), 0.0f); in[480] = 1.0f;                // impulse at 10 ms
    const Vec w = wetOf(run(e, in), in);
    size_t arg = 0; for (size_t i = 0; i < w.size(); ++i) if (std::abs(w[i]) > std::abs(w[arg])) arg = i;
    const double lag = double(arg - 480) / fs;
    INFO("tracking " << t << " lag " << lag << " expected " << expected);
    CHECK(std::abs(lag - expected) <= 2.0 / fInt + 1e-3);           // 2 internal samples + 1 ms
    CHECK(lag > prevLag); prevLag = lag;                            // longer lag as Tracking decreases
  }
}

// T-009 [unit/audio] Secondary voice: octave above/below per Pitch side; chorus voice at noon. Enforces: SC-2, C-007, D-006.
TEST_CASE("T-009 secondary voice octave logic", "[audio][T-009]") {
  const double fs = 48000; const Vec in = sine(440, fs, 1.5);
  for (float k : {0.0f, 0.25f, 0.75f, 1.0f}) {
    const Vec w = wetFor(fs, k, 0.0f, 1.0f, in);
    const double oct = k > 0.5f ? 2.0 : 0.5;
    const double f = dominantHz(w, fs, size_t(0.5 * fs));
    INFO("knob " << k << " measured " << f);
    CHECK(std::abs(f / (440.0 * oct * ratioOf(expectedSemitones(k))) - 1.0) <= kRatioTolOct);
  }
  const Vec w = wetFor(fs, 0.5f, 0.0f, 1.0f, in);
  CHECK(std::abs(dominantHz(w, fs, size_t(0.5 * fs)) / 440.0 - 1.0) <= kRatioTolPedal);
  CHECK(rms(w, size_t(0.5 * fs)) > 0.05);                           // voice present at noon
}

// T-010 [unit/audio] Noon acts as a chorus: moving detune bounded by kChorusDepthCents. Enforces: C-011, D-012.
TEST_CASE("T-010 noon chorus detune", "[audio][T-010]") {
  const double fs = 48000; const Vec in = sine(440, fs, 3.0);
  const Vec w = wetFor(fs, 0.5f, 1.0f, 0.0f, in);
  double lo = 1e9, hi = -1e9;
  for (int s = 0; s < 8; ++s) {
    const size_t b = size_t((0.5 + 0.25 * s) * fs), e = b + size_t(0.25 * fs);
    const double cents = 1200.0 * std::log2(dominantHz(w, fs, b, e) / 440.0);
    lo = std::min(lo, cents); hi = std::max(hi, cents);
    CHECK(std::abs(cents) <= calib::kChorusDepthCents + 5.0);
  }
  CHECK(hi - lo >= 1.0);
}

static Vec trail(float pitchKnob) {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, pitchKnob); e.setParameter(ParamId::Primary, calib::kUnityKnob);
  e.setParameter(ParamId::Magic, 0.6f); e.setParameter(ParamId::MagicEngaged, 1.0f); e.reset();
  Vec in(size_t(1.0 * fs), 0.0f); const Vec b = sine(300, fs, 0.03); std::copy(b.begin(), b.end(), in.begin());
  const Vec w = wetOf(run(e, in), in);
  Vec freqs; const size_t seg = size_t(0.04 * fs);
  for (size_t i = b.size(); i + seg < w.size(); i += seg) if (rms(w, i, i + seg) > 1e-3) freqs.push_back(float(dominantHz(w, fs, i, i + seg)));
  return freqs;
}
// T-011 [unit/audio] Magic trails ascend above noon, descend below. Enforces: SC-2, C-015, C-021, D-008.
TEST_CASE("T-011 magic pitch trails direction", "[audio][T-011]") {
  const Vec up = trail(1.0f), down = trail(0.0f);
  REQUIRE(up.size() >= 3); REQUIRE(down.size() >= 3);
  CHECK(up[0] < up[1]); CHECK(up[1] < up[2]);
  CHECK(down[0] > down[1]); CHECK(down[1] > down[2]);
}

// T-012 [unit/audio] Magic disengaged: no regeneration after window + lag. Enforces: C-006, D-008.
TEST_CASE("T-012 no regeneration when magic off", "[audio][T-012]") {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, 1.0f); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Secondary, 1.0f);
  e.setParameter(ParamId::Magic, 1.0f); e.setParameter(ParamId::MagicEngaged, 0.0f); e.reset();
  Vec in(size_t(1.0 * fs), 0.0f); const Vec b = sine(300, fs, 0.1); std::copy(b.begin(), b.end(), in.begin());
  const Vec w = wetOf(run(e, in), in);
  const double settle = 0.1 + 2.0 * calib::kWindowSamples / calib::kNominalClockHz + 0.05;
  CHECK(rms(w, size_t(settle * fs)) < 1e-5);
}

// T-013 [unit/audio] Max Magic self-oscillates but stays bounded and finite. Enforces: C-006, D-008, R-004.
// Bound: |dry|<=0.25; loop soft-clipped (tanh) so |shifter in|<=0.25+kMagicMaxFeedback; two voices at gain 1/kUnityKnob.
TEST_CASE("T-013 bounded self-oscillation", "[audio][T-013]") {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, 0.8f); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Secondary, 1.0f);
  e.setParameter(ParamId::Magic, 1.0f); e.setParameter(ParamId::MagicEngaged, 1.0f); e.setParameter(ParamId::Tracking, 0.0f); e.reset();
  Vec in(size_t(10.0 * fs), 0.0f); const Vec b = sine(300, fs, 0.05); std::copy(b.begin(), b.end(), in.begin());
  const Vec out = run(e, in);
  const float bound = 0.25f + 2.0f * (0.25f + calib::kMagicMaxFeedback) / calib::kUnityKnob;
  bool finite = true; float pk = 0; for (float x : out) { finite &= std::isfinite(x); pk = std::max(pk, std::abs(x)); }
  CHECK(finite); CHECK(pk <= bound);
  CHECK(rms(out, size_t(9.0 * fs)) > 1e-3);                         // still oscillating at 9 s (self-oscillation)
}

// T-014 [unit/audio] Tone rolls off treble on the wet signal. Enforces: C-005, D-007.
// First-order roll-off at 1 kHz gives >=12.3 dB at 4 kHz; 10 dB threshold leaves margin for resampler images.
TEST_CASE("T-014 tone darkens wet", "[audio][T-014]") {
  const double fs = 48000; const Vec in = noise(size_t(2 * fs), 99u);
  auto hf = [&](float tone) { Engine e; e.prepare(fs, 512); neutral(e); e.setParameter(ParamId::Primary, 1.0f);
    e.setParameter(ParamId::Tone, tone); e.reset(); const Vec w = wetOf(run(e, in), in); return bandEnergy(w, fs, 4000, 12000, size_t(0.5 * fs)); };
  CHECK(10.0 * std::log10(hf(1.0f) / hf(0.0f)) >= 10.0);
}

// T-015 [unit/audio] Bypass: exact passthrough after fade, no click on toggle. Enforces: SC-4, C-008, D-013.
TEST_CASE("T-015 bypass exactness and click-free toggle", "[audio][T-015]") {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, 0.75f); e.setParameter(ParamId::Primary, 1.0f); e.reset();
  const Vec in = sine(440, fs, 1.0); Vec out(in.size());
  const size_t half = size_t(0.5 * fs);
  for (size_t p = 0; p < in.size(); p += 128) {
    if (p == half) e.setParameter(ParamId::Bypass, 1.0f);
    const float* i[1] = { in.data() + p }; float* o[1] = { out.data() + p }; e.process(i, o, 1, 1, int(std::min<size_t>(128, in.size() - p)));
  }
  const size_t after = half + size_t((calib::kBypassFadeSeconds + 0.002) * fs);
  CHECK(maxAbsDiff(Vec(out.begin() + long(after), out.end()), Vec(in.begin() + long(after), in.end())) == 0.0f);
  const float steady = maxStep(out, half - size_t(0.2 * fs), half);
  CHECK(maxStep(out, half, half + size_t(0.02 * fs)) <= 1.5f * steady + 0.02f);
}

// T-016 [unit/audio] Parameter steps are smoothed (no zipper/clicks). Enforces: D-014.
TEST_CASE("T-016 parameter smoothing", "[audio][T-016]") {
  const double fs = 48000; const Vec in = sine(220, fs, 1.0);
  for (ParamId id : {ParamId::Pitch, ParamId::Primary, ParamId::Secondary, ParamId::Tracking, ParamId::Tone, ParamId::Magic}) {
    Engine e; e.prepare(fs, 512); neutral(e);
    e.setParameter(ParamId::Primary, 0.5f); e.setParameter(ParamId::Secondary, 0.3f); e.setParameter(ParamId::MagicEngaged, 1.0f);
    e.setParameter(id, 0.0f); e.reset();
    Vec out(in.size()); const size_t half = size_t(0.5 * fs);
    for (size_t p = 0; p < in.size(); p += 128) {
      if (p == half) e.setParameter(id, 1.0f);
      const float* i[1] = { in.data() + p }; float* o[1] = { out.data() + p }; e.process(i, o, 1, 1, 128);
    }
    INFO("param " << int(id));
    CHECK(maxStep(out, half, half + 256) <= 1.5f * maxStep(out, half - size_t(0.2 * fs), half) + 0.05f);
  }
}

// T-017 [unit/audio] Expression input replaces Pitch when enabled. Enforces: SC-4, C-002, D-009.
TEST_CASE("T-017 expression pedal overrides pitch", "[audio][T-017]") {
  const double fs = 48000; const Vec in = sine(440, fs, 1.5);
  auto measure = [&](float expOn) { Engine e; e.prepare(fs, 512); neutral(e); e.setParameter(ParamId::Primary, 1.0f);
    e.setParameter(ParamId::Pitch, 0.0f); e.setParameter(ParamId::PitchExp, 1.0f); e.setParameter(ParamId::ExpEnabled, expOn); e.reset();
    return dominantHz(wetOf(run(e, in), in), fs, size_t(0.5 * fs)); };
  CHECK(std::abs(measure(1.0f) / (440.0 * ratioOf(expectedSemitones(1.0f))) - 1.0) <= kRatioTolPedal);
  CHECK(std::abs(measure(0.0f) / (440.0 * ratioOf(expectedSemitones(0.0f))) - 1.0) <= kRatioTolPedal);
}

// T-019 [unit/audio] Sample-rate invariance of lag and ratio. Enforces: SC-6, D-010.
TEST_CASE("T-019 sample-rate invariance", "[audio][T-019]") {
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0, 192000.0}) {
    const Vec in = sine(440, fs, 1.2);
    const Vec w = wetFor(fs, 1.0f, 1.0f, 0.0f, in);
    INFO("fs " << fs);
    CHECK(std::abs(dominantHz(w, fs, size_t(0.4 * fs)) / (440.0 * ratioOf(expectedSemitones(1.0f))) - 1.0) <= kRatioTolPedal);
    Engine e; e.prepare(fs, 512); neutral(e); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Tracking, 0.5f); e.reset();
    Vec imp(size_t(0.4 * fs), 0.0f); imp[100] = 1.0f; const Vec wi = wetOf(run(e, imp), imp);
    size_t arg = 0; for (size_t i = 0; i < wi.size(); ++i) if (std::abs(wi[i]) > std::abs(wi[arg])) arg = i;
    CHECK(std::abs(double(arg - 100) / fs - e.nominalWetLagSeconds()) <= 1e-3 + 2.0 / (calib::kNominalClockHz * calib::kClockScaleMin));
  }
}
