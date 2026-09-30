#include "dsp/Mapping.hpp"
#include <cmath>
#include <limits>
#include "dsp/Calibration.hpp"
namespace refractor::mapping {
namespace {
// Out-of-range inputs clamp to [0,1]; non-finite inputs fall back to the parameter default (Mapping.hpp contract).
double sanitise(float knob, double fallback) {
  if (!std::isfinite(knob)) return fallback;
  return knob < 0.0f ? 0.0 : (knob > 1.0f ? 1.0 : static_cast<double>(knob));
}
}  // namespace

float pitchKnobToSemitones(float knob) {
  const double k = sanitise(knob, 0.5);
  const double s = k <= 0.5 ? calib::kPitchDownSemitones * (1.0 - 2.0 * k)
                            : calib::kPitchUpSemitones * (2.0 * k - 1.0);
  return static_cast<float>(s);
}
float semitonesToRatio(float semitones) { return static_cast<float>(std::pow(2.0, static_cast<double>(semitones) / 12.0)); }
float levelKnobToGain(float knob) { return static_cast<float>(sanitise(knob, calib::kUnityKnob) / calib::kUnityKnob); }
double trackingToClockScale(float knob) {
  const double k = sanitise(knob, 0.8);
  return calib::kClockScaleMin * std::pow(calib::kClockScaleMax / calib::kClockScaleMin, k);
}
float toneToCutoffHz(float knob) {
  const double k = sanitise(knob, 1.0);
  if (k >= 1.0) return std::numeric_limits<float>::infinity();
  return static_cast<float>(calib::kToneMinCutoffHz * std::pow(20000.0 / calib::kToneMinCutoffHz, k));
}
float magicToFeedback(float knob) { return static_cast<float>(sanitise(knob, 0.3) * calib::kMagicMaxFeedback); }
}  // namespace refractor::mapping
