# Amendments to the frozen plan (2026-09-30)

## Why
During S-006 the delay-line pitch shifter of the original D-010 was found to mistune its output by an amount that depended on the
input frequency (spike S3: up to +21 % at 220 Hz; 0.6 % only at the 440 Hz the planning spike happened to use — research/ERRATA.md
E-001). Independently, three frozen tests were defective (TEST_CHALLENGE-RESOLVED.md TC-1…TC-3) and one tolerance had no evidence
(TC-4). Options A–D were put to the owner (BLOCKED-RESOLVED.md).

## Owner decision (verbatim): "1c and 2 - ok to change the tests"
= option C (STFT phase-vocoder shifter) and approval to amend the frozen tests.

## What changed
| Area | Before | After |
|---|---|---|
| D-010 shifter | two-tap delay line, lag 512/f_int | phase vocoder, identity phase locking, lag N/f_int (N = kWindowSamples = 1024), alias folding |
| D-008 | tanh loop only | + soft wet ceiling kWetCeiling = 4.0 (|out| ≤ |dry| + 4) |
| ClockDomain input | point-sampled linear interpolation | area average over one tick (DEVIATIONS D-A) |
| T-015 / T-016 | toggle at `p == half` (unreachable) | `p >= half && p - half < 128` |
| T-018 | final 128-sample block overran 24000-sample buffers | block clamped to remaining samples |
| T-009 | octave tolerance 2.5 % | 1.0 % (pitch error now ≈ 0; budget covers the chorus LFO) |
| T-008, T-019 (lag part) | single-sample impulse peak = 512/f_int | energy centroid of a Hann-windowed 330 Hz burst = N/f_int (noise bursts jitter ±2 ms: S4d) |
| T-013 | bound 0.25 + 2·(0.25+1.15)/0.7 | bound 0.25 + kWetCeiling (crest factor 1.7× the old bound: S4e) |
| T-025 (new) | — | pitch accuracy ≤ 1 % for 9 input frequencies × 4 knob positions |
| T-026 (new) | — | shifted chords keep ≥ 90 % of wet energy on their partials (Tracking 0) |
All other frozen files are byte-identical (hash comparison when the manifest was regenerated).

## Verification after the change (Ubuntu 24.04, GCC 13.3, Release)
- dsp_tests: 26/26 cases, 1336 assertions pass; also clean under ASan + UBSan.   rt_tests (no allocation): pass.
- Bench worst case (Tracking 1, both voices, Magic max): 2.3 % (48 kHz/128), 2.5 % (192 kHz/128) of one core; frozen bench (Tracking 0): 0.42 % / 0.71 %.
- Reproduce spike numbers: research/spikes/S4-phase-vocoder/README.md.

## Known limitation accepted with option C (R-018)
Lag × frequency resolution = 1: at Tracking 1 (lag 31 ms, 32 Hz bins) close-spaced chord partials smear (G triad 42 % clean);
below Tracking ≈ 0.6 chords are clean. `kWindowSamples` = 2048 halves the smearing and doubles the lag — decide at G-003.
