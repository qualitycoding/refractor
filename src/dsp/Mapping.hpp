#pragma once
// Knob-to-physical mappings (D-004..D-009). All inputs are normalised knob positions in [0,1];
// out-of-range inputs are clamped to [0,1]; non-finite inputs are treated as the parameter default.
namespace refractor::mapping {
float pitchKnobToSemitones(float knob);   // 0 -> kPitchDownSemitones, 0.5 -> 0, 1 -> kPitchUpSemitones; piecewise linear
float semitonesToRatio(float semitones);  // 2^(s/12)
float levelKnobToGain(float knob);        // linear amplitude: knob / kUnityKnob
double trackingToClockScale(float knob);  // exponential: kClockScaleMin * (kClockScaleMax/kClockScaleMin)^knob
float toneToCutoffHz(float knob);         // exponential kToneMinCutoffHz..20000 Hz; knob==1 -> +infinity (bypass)
float magicToFeedback(float knob);        // knob * kMagicMaxFeedback
}
