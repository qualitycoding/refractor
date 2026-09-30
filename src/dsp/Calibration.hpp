#pragma once
// Calibration constants (D-011). These are DATA, not stubs. They may be changed ONLY at gate G-003
// (listening sign-off), and only within the static_assert bounds below. Tests read them symbolically.
namespace refractor::calib {
inline constexpr double kNominalClockHz      = 32768.0; // FV-1-class nominal internal rate (C-012)
inline constexpr int    kWindowSamples       = 1024;    // phase-vocoder FFT size = Tracking lag in internal samples (power of two)
inline constexpr int    kOverlap             = 4;       // frames per window (hop = kWindowSamples / kOverlap)
inline constexpr double kClockScaleMin       = 0.15;    // Tracking fully CCW
inline constexpr double kClockScaleMax       = 1.0;     // Tracking fully CW
inline constexpr float  kPitchDownSemitones  = -5.0f;   // "a fourth below" (C-003)
inline constexpr float  kPitchUpSemitones    =  4.0f;   // "a third above", major third (A-013)
inline constexpr float  kUnityKnob           = 0.7f;    // level knob position giving unity gain ("2 o'clock", A-014)
inline constexpr float  kChorusDepthCents    = 8.0f;    // detune LFO depth, always on (D-012)
inline constexpr float  kChorusRateHz        = 0.6f;
inline constexpr float  kToneMinCutoffHz     = 1000.0f; // Tone fully CCW
inline constexpr float  kMagicMaxFeedback    = 1.15f;   // >1 permits controllable self-oscillation (C-006)
inline constexpr float  kWetCeiling         = 4.0f;    // hard bound on the wet signal (+12 dBFS); linear below half (R-004)
inline constexpr double kSmoothingSeconds    = 0.02;    // parameter smoothing time constant
inline constexpr double kBypassFadeSeconds   = 0.01;
inline constexpr double kHoldSeconds    = 0.30;    // hold time distinguishing momentary from latching
inline constexpr float  kFlushThreshold      = 1e-15f;  // |x| below this in feedback/state is flushed to 0

static_assert(kWindowSamples >= 512 && kWindowSamples <= 4096 && (kWindowSamples & (kWindowSamples - 1)) == 0);
static_assert(kWetCeiling >= 2.0f && kWetCeiling <= 8.0f);
static_assert(kOverlap >= 4 && kOverlap <= 8);
static_assert(kClockScaleMin >= 0.10 && kClockScaleMin <= 0.50);
static_assert(kClockScaleMax == 1.0);
static_assert(kChorusDepthCents >= 2.0f && kChorusDepthCents <= 20.0f);
static_assert(kMagicMaxFeedback >= 0.9f && kMagicMaxFeedback <= 1.5f);
static_assert(kToneMinCutoffHz >= 500.0f && kToneMinCutoffHz <= 3000.0f);
}
