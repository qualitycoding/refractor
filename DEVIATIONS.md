# Deviations

## 2026-09-30 D-A — Clock-domain input is area-averaged, not point-sampled (touches D-010 wording; engine-internal)
S-006: with point sampling at low Tracking (internal tick every ~10 host samples) a one-sample transient is skipped entirely
when it falls between ticks, so T-008 (Tracking 0) and T-019 could not find the wet impulse. ClockDomain now averages the
piecewise-linear host signal over the time since the previous tick. This is a weak low-pass, not an anti-alias filter;
aliasing remains (C-013). Reversible: `src/dsp/ClockDomain.ipp`. Listen for it at G-003.

## 2026-09-30 D-B — D-006 octave "crossfade" implemented as a 20 ms glide
The secondary voice's octave factor (−1, 0, +1 in log2 domain) is smoothed with the standard one-pole instead of crossfading
between separate octave voices (would need a third shifter). Click-free; audibly a short pitch sweep of the secondary voice
when Pitch crosses noon. Reversible; listen at G-003.

## 2026-09-30 D-C — Build directories outside the repository
The planning sandbox reused /home/claude/build-dsp and /home/claude/build-full (FetchContent cache /home/claude/fc) to avoid
a ~20-minute JUCE rebuild on 1 vCPU. No effect on the repository.
