#include "dsp/Engine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include "dsp/Calibration.hpp"
#include "dsp/ClockDomain.hpp"
#include "dsp/Mapping.hpp"
#include "dsp/PitchShifter.hpp"

namespace refractor {
namespace {
constexpr double kTwoPi = 2.0 * std::numbers::pi;
constexpr double kOctaveDeadband = 0.01;                 // D-006
constexpr double kToneBypassHz = 19900.0;                // D-007
constexpr double kToneInfTargetHz = 24000.0;
inline float flush(float x) { return std::fabs(x) < calib::kFlushThreshold ? 0.0f : x; }
// Transparent below kWetCeiling/2; smoothly saturates to +-kWetCeiling (continuous value and slope at the knee).
inline float softCeiling(float x) {
  constexpr float T = calib::kWetCeiling, K = 0.5f * T;
  const float a = std::fabs(x);
  if (a <= K) return x;
  const float y = K + K * std::tanh((a - K) / K);
  return x < 0.0f ? -y : y;
}
inline float sane(float x) { return std::isfinite(x) ? x : 0.0f; }
inline void smooth(double& x, double target, double a) {
  x += a * (target - x);
  if (std::fabs(target - x) < 1e-9 * (1.0 + std::fabs(target))) x = target;
}
}  // namespace

struct Engine::Impl {
  static constexpr std::size_t N = static_cast<std::size_t>(ParamId::Count);
  double fs = 48000.0, smoothA = 0.0, fbSmoothA = 0.0, bypassStep = 1.0;
  std::array<float, N> params{};
  // targets (derived from params in updateTargets)
  double tSemis = 0, tGP = 0, tGS = 0, tFb = 0, tLogFc = 0, tOct = 0, tBypass = 0, tTracking = 0.8;
  bool tToneInf = true;
  // smoothed state
  double semis = 0, gP = 0, gS = 0, fb = 0, logFc = 0, oct = 0, byp = 0, lastLogFc = -1e300;
  float toneCoef = 1.0f;
  bool wasBypassed = false;
  // effect state
  ClockDomain clock;
  PhaseVocoderShifter shP, shS;
  double lfoPhase = 0.0;
  float fbState = 0.0f, toneY = 0.0f;

  Impl() {
    for (const auto& s : parameterSpecs()) params[static_cast<std::size_t>(s.id)] = s.defaultValue;
    prepare(48000.0);
  }
  void prepare(double sampleRate) {
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    smoothA = 1.0 - std::exp(-1.0 / (calib::kSmoothingSeconds * fs));
    fbSmoothA = 1.0 - std::exp(-1.0 / (calib::kBypassFadeSeconds / 3.0 * fs));
    bypassStep = 1.0 / std::max(1.0, std::round(calib::kBypassFadeSeconds * fs));
    clock.prepare(fs, 0);
    shP.prepare(calib::kWindowSamples, calib::kOverlap);
    shS.prepare(calib::kWindowSamples, calib::kOverlap);
    updateTargets();
    reset();
  }
  void updateTargets() {
    auto P = [&](ParamId id) { return params[static_cast<std::size_t>(id)]; };
    const float knob = P(ParamId::ExpEnabled) >= 0.5f ? P(ParamId::PitchExp) : P(ParamId::Pitch);   // D-009
    tSemis = mapping::pitchKnobToSemitones(knob);
    tOct = knob > 0.5 + kOctaveDeadband ? 1.0 : (knob < 0.5 - kOctaveDeadband ? -1.0 : 0.0);        // D-006
    tGP = mapping::levelKnobToGain(P(ParamId::Primary));
    tGS = mapping::levelKnobToGain(P(ParamId::Secondary));
    tFb = P(ParamId::MagicEngaged) >= 0.5f ? mapping::magicToFeedback(P(ParamId::Magic)) : 0.0;     // D-008
    const float fc = mapping::toneToCutoffHz(P(ParamId::Tone));
    tToneInf = std::isinf(fc);
    tLogFc = std::log(tToneInf ? kToneInfTargetHz : static_cast<double>(fc));
    tBypass = P(ParamId::Bypass) >= 0.5f ? 1.0 : 0.0;
    tTracking = P(ParamId::Tracking);
    clock.setClockScale(mapping::trackingToClockScale(static_cast<float>(tTracking)));
  }
  void clearEffectState() {
    shP.reset(); shS.reset(); clock.reset();
    lfoPhase = 0.0; fbState = 0.0f; toneY = 0.0f;
  }
  void reset() {
    semis = tSemis; gP = tGP; gS = tGS; fb = tFb; logFc = tLogFc; oct = tOct; byp = tBypass;
    lastLogFc = -1e300;
    wasBypassed = byp >= 1.0;
    clearEffectState();
  }

  // One internal-clock tick: both voices + Magic regeneration loop (D-002, D-008, D-010, D-012).
  float tickInternal(float x) {
    lfoPhase += kTwoPi * calib::kChorusRateHz / clock.internalRate();
    if (lfoPhase >= kTwoPi) lfoPhase -= kTwoPi;
    const double depth = calib::kChorusDepthCents / 1200.0;
    const double rP = std::exp2(semis / 12.0 + depth * std::sin(lfoPhase));
    const double rS = std::exp2(semis / 12.0 + oct + depth * std::cos(lfoPhase));   // quadrature LFO, octave glide
    const float u = flush(x + static_cast<float>(fb) * fbState);
    shP.setRatio(static_cast<float>(rP));
    shS.setRatio(static_cast<float>(rS));
    const float yP = shP.processSample(u);
    const float yS = shS.processSample(u);
    const float w = flush(static_cast<float>(gP) * yP + static_cast<float>(gS) * yS);
    fbState = flush(std::tanh(w));          // regeneration uses the un-limited wet
    return softCeiling(w);                   // output is bounded by kWetCeiling (D-008)
  }

  void process(const float* const* in, float* const* out, int numIn, int numOut, int numSamples) {
    numIn = std::clamp(numIn, 1, 2);
    const int outCh = std::clamp(numOut, 0, 2);
    const double toneBypassLog = std::log(kToneBypassHz);
    for (int n = 0; n < numSamples; ++n) {
      const float x0 = sane(in[0][n]);
      const float x1 = numIn > 1 ? sane(in[1][n]) : 0.0f;
      const float mono = numIn > 1 ? 0.5f * (x0 + x1) : x0;
      float dry0, dry1;
      if (numIn == 2 && outCh == 2) { dry0 = x0; dry1 = x1; }
      else if (numIn == 2)          { dry0 = mono; dry1 = mono; }
      else                          { dry0 = x0; dry1 = x0; }

      smooth(semis, tSemis, smoothA); smooth(gP, tGP, smoothA); smooth(gS, tGS, smoothA);
      smooth(oct, tOct, smoothA);     smooth(logFc, tLogFc, smoothA);
      fb += fbSmoothA * (tFb - fb);
      if (tFb == 0.0 && fb < 1e-6) fb = 0.0; else if (std::fabs(tFb - fb) < 1e-9) fb = tFb;
      if (byp < tBypass) byp = std::min(byp + bypassStep, tBypass);
      else if (byp > tBypass) byp = std::max(byp - bypassStep, tBypass);

      float wet = 0.0f;
      if (byp >= 1.0) {
        if (!wasBypassed) { clearEffectState(); wasBypassed = true; }   // true-bypass: cut tails (D-013)
      } else {
        wasBypassed = false;
        wet = clock.tick(mono, [this](float xin) { return tickInternal(xin); });
        const bool toneOff = tToneInf && logFc >= toneBypassLog;
        if (toneOff) toneY = wet;
        else {
          if (logFc != lastLogFc) {
            toneCoef = static_cast<float>(1.0 - std::exp(-kTwoPi * std::exp(logFc) / fs));
            lastLogFc = logFc;
          }
          toneY = flush(toneY + toneCoef * (wet - toneY));
          wet = toneY;
        }
      }
      const float wetScale = static_cast<float>(1.0 - byp);
      if (outCh > 0) out[0][n] = byp >= 1.0 ? dry0 : dry0 + wet * wetScale;
      if (outCh > 1) out[1][n] = byp >= 1.0 ? dry1 : dry1 + wet * wetScale;
    }
  }
};

Engine::Engine() : impl_(std::make_unique<Impl>()) {}
Engine::~Engine() = default;
void Engine::prepare(double sampleRate, int) { impl_->prepare(sampleRate); }
void Engine::reset() { impl_->reset(); }
void Engine::setParameter(ParamId id, float value) {
  const auto i = static_cast<std::size_t>(id);
  if (i >= Impl::N) return;
  const ParamSpec& s = spec(id);
  float v = std::isfinite(value) ? std::clamp(value, s.min, s.max) : s.defaultValue;
  if (s.isBool) v = v >= 0.5f ? 1.0f : 0.0f;
  impl_->params[i] = v;
  impl_->updateTargets();
}
float Engine::getParameter(ParamId id) const {
  const auto i = static_cast<std::size_t>(id);
  return i < Impl::N ? impl_->params[i] : 0.0f;
}
void Engine::process(const float* const* in, float* const* out, int numIn, int numOut, int numSamples) {
  impl_->process(in, out, numIn, numOut, numSamples);
}
double Engine::nominalWetLagSeconds() const {
  return static_cast<double>(impl_->shP.latencySamples()) /
         (calib::kNominalClockHz * mapping::trackingToClockScale(static_cast<float>(impl_->tTracking)));
}
}  // namespace refractor
