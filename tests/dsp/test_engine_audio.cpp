// FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256, plan/DECISIONS.md "Immutability")
// Audio-behaviour tests of refractor::Engine.
// AMENDED 2026-09-30 (owner-approved, plan/AMENDMENTS.md): the delay-line shifter of the original plan mistuned its output
// by several percent at most input frequencies (spike S3); the engine now uses a phase-vocoder shifter whose measured
// pitch error is <0.01% at every tested frequency and ratio (spike S4, research/spikes/S4-phase-vocoder). The remaining
// tolerance budget covers only the always-on chorus LFO (+-8 cents = +-0.46% instantaneous, <=0.3% over the analysis windows).
// Lag is now N/f_int (energy-centroid definition); T-015/T-016/T-018 toggle/loop arithmetic fixed; T-013 bound is the wet
// ceiling; T-025/T-026 are new (any-frequency accuracy, chord fidelity).
#include <catch2/catch_test_macros.hpp>
#include <array>
#include <cmath>
#include <numbers>
#include "support/TestSupport.hpp"
#include "dsp/Mapping.hpp"
using namespace refractor; using namespace rt;

static constexpr double kRatioTolPedal = 0.010;  // 1.0 %  (~17 cents)
static constexpr double kRatioTolOct   = 0.010;  // 1.0 %  (was 2.5 % for the delay-line shifter)

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

// Lag = energy-centroid delay between the dry and the wet Hann-windowed 330 Hz tone burst (an STFT shifter has no meaningful
// single-sample impulse response, so the original impulse-peak definition was replaced). A tone is used rather than noise
// because the centroid of a random burst jitters by +-2 ms with the seed (spike S4d); for the tone it agrees with
// kWindowSamples/f_int to <0.2 ms at every Tracking setting.
static double measuredLagSeconds(double fs, float tracking) {
  Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Tracking, tracking); e.reset();
  const double lagS = e.nominalWetLagSeconds();
  const size_t t0 = size_t(0.05 * fs), L = size_t(0.3 * fs), total = t0 + L + size_t((2.5 * lagS + 0.1) * fs);
  Vec in(total, 0.0f); const Vec nz = sine(330.0, fs, double(L) / fs, 0.5f);
  for (size_t i = 0; i < L; ++i) in[t0 + i] = nz[i] * float(0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * double(i) / double(L - 1)));
  const Vec w = wetOf(run(e, in), in);
  auto centroid = [](const Vec& v) { double a = 0, b = 0; for (size_t i = 0; i < v.size(); ++i) { a += double(v[i]) * v[i] * double(i); b += double(v[i]) * v[i]; } return a / b; };
  return (centroid(w) - centroid(in)) / fs;
}

// T-008 [unit/audio] Wet lag follows Tracking via the internal clock: lag = kWindowSamples / f_int. Enforces: SC-2, C-009, C-010, C-022, D-010.
TEST_CASE("T-008 wet lag versus tracking", "[audio][T-008]") {
  const double fs = 48000; double prevLag = -1.0;
  for (float t : {1.0f, 0.75f, 0.5f, 0.25f, 0.0f}) {
    Engine e; e.prepare(fs, 512); neutral(e); e.setParameter(ParamId::Tracking, t); e.reset();
    const double fInt = calib::kNominalClockHz * mapping::trackingToClockScale(t);
    const double expected = double(calib::kWindowSamples) / fInt;
    CHECK(std::abs(e.nominalWetLagSeconds() - expected) <= 1e-9);
    const double lag = measuredLagSeconds(fs, t);
    INFO("tracking " << t << " lag " << lag << " expected " << expected);
    CHECK(std::abs(lag - expected) <= 3.0 / fInt + 5e-4);           // 3 internal samples (resampling) + 0.5 ms
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
// Bound (AMENDED): the wet signal is limited to kWetCeiling by design (D-008), so |out| <= |dry| + kWetCeiling. The old analytic
// bound assumed a convex (delay-line) shifter; a phase vocoder has crest factor (measured up to 1.7x that bound).
TEST_CASE("T-013 bounded self-oscillation", "[audio][T-013]") {
  const double fs = 48000; Engine e; e.prepare(fs, 512); neutral(e);
  e.setParameter(ParamId::Pitch, 0.8f); e.setParameter(ParamId::Primary, 1.0f); e.setParameter(ParamId::Secondary, 1.0f);
  e.setParameter(ParamId::Magic, 1.0f); e.setParameter(ParamId::MagicEngaged, 1.0f); e.setParameter(ParamId::Tracking, 0.0f); e.reset();
  Vec in(size_t(10.0 * fs), 0.0f); const Vec b = sine(300, fs, 0.05); std::copy(b.begin(), b.end(), in.begin());
  const Vec out = run(e, in);
  const float bound = 0.25f + calib::kWetCeiling;
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
    if (p >= half && p - half < 128) e.setParameter(ParamId::Bypass, 1.0f);   // AMENDED (TC-1): p is a multiple of 128, half is not
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
      if (p >= half && p - half < 128) e.setParameter(id, 1.0f);   // AMENDED (TC-2): was unreachable (vacuous test)
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

// T-019 [unit/audio] Sample-rate invariance of lag (centroid definition) and ratio. Enforces: SC-6, D-010.
TEST_CASE("T-019 sample-rate invariance", "[audio][T-019]") {
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0, 192000.0}) {
    const Vec in = sine(440, fs, 1.2);
    const Vec w = wetFor(fs, 1.0f, 1.0f, 0.0f, in);
    INFO("fs " << fs);
    CHECK(std::abs(dominantHz(w, fs, size_t(0.4 * fs)) / (440.0 * ratioOf(expectedSemitones(1.0f))) - 1.0) <= kRatioTolPedal);
    const double lag = measuredLagSeconds(fs, 0.5f);
    const double expected = double(calib::kWindowSamples) / (calib::kNominalClockHz * mapping::trackingToClockScale(0.5f));
    CHECK(std::abs(lag - expected) <= 5e-4 + 3.0 / (calib::kNominalClockHz * calib::kClockScaleMin));
  }
}

// T-025 [unit/audio] Pitch ratio is accurate for ANY input frequency (NEW 2026-09-30). Regression for the defect found in
// S-006: the delay-line shifter was off by up to +21 % depending on input frequency (spike S3); the same sweep on the
// phase vocoder shows <0.01 % (spike S4). Enforces: SC-2, C-003, C-023, D-010.
TEST_CASE("T-025 pitch accuracy across input frequency", "[audio][T-025]") {
  const double fs = 48000;
  for (double f0 : {82.0, 110.0, 165.0, 220.0, 330.0, 440.0, 660.0, 880.0, 1200.0}) {
    const Vec in = sine(f0, fs, 2.0);
    for (float k : {0.0f, 0.25f, 0.75f, 1.0f}) {
      const Vec w = wetFor(fs, k, 1.0f, 0.0f, in);
      const double f = dominantHz(w, fs, size_t(0.5 * fs));
      INFO("input " << f0 << " Hz, knob " << k << ", measured " << f);
      CHECK(std::abs(f / (f0 * ratioOf(expectedSemitones(k))) - 1.0) <= kRatioTolPedal);
    }
  }
}

// T-026 [unit/audio] Polyphony: a shifted chord keeps its partials (NEW). At Tracking 0 the analysis bin is f_int/N ~ 5 Hz,
// so >=90 % of the wet energy must sit within +-3 % of the three shifted partials (spike S4c: 100 %). Enforces: SC-2, C-024, D-010.
TEST_CASE("T-026 chord fidelity", "[audio][T-026]") {
  const double fs = 48000;
  for (const auto& f : {std::array<double, 3>{196.0, 247.0, 294.0}, std::array<double, 3>{82.4, 123.5, 164.8}}) {
    Vec in(size_t(3.0 * fs), 0.0f);
    for (double fr : f) { const Vec s = sine(fr, fs, 3.0, 0.15f); for (size_t i = 0; i < in.size(); ++i) in[i] += s[i]; }
    Engine e; e.prepare(fs, 512); neutral(e);
    e.setParameter(ParamId::Pitch, 0.75f); e.setParameter(ParamId::Primary, calib::kUnityKnob); e.setParameter(ParamId::Tracking, 0.0f); e.reset();
    const Vec w = wetOf(run(e, in), in); const double r = ratioOf(expectedSemitones(0.75f)); const size_t b = size_t(1.2 * fs);
    const double total = bandEnergy(w, fs, 20, 16000, b); double on = 0;
    for (double fr : f) on += bandEnergy(w, fs, 0.97 * fr * r, 1.03 * fr * r, b);
    INFO("chord root " << f[0] << " Hz: on-partial fraction " << on / total);
    CHECK(on / total >= 0.90);
  }
}
