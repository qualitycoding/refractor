# Decisions, interfaces and decision rules (D-###)

## Architecture & toolchain

**D-001 Framework: JUCE 8.0.15 (commit `91ad83ae34a81e0833b1a2b0866f54846370ae53`), licence AGPLv3.**
JUCE 7.0.12's `juceaide` does not compile against the macOS 15 SDK / Xcode 16 (C-030) and GitHub's `macos-14` image is
deprecated (C-033), so JUCE 7 cannot meet SC-1. JUCE 9.0.x is <3 months old; 8.0.15 is the last 8.x release (C-031).
Consequence: Refractor is licensed **AGPL-3.0-only** (JUCE's open-source option, C-031). Supersedes A-007.
*Rule:* if any CI platform fails to build JUCE 8.0.15 for a reason inside JUCE → try 9.0.3 (commit from `git ls-remote`,
added to ENVIRONMENT.md lock table, T-050 re-run) on a branch; if that passes all frozen tests, adopt it and log in
DEVIATIONS.md; otherwise halt with BLOCKED.md.

**D-002 Signal flow (V2 behavioural model).**
```
in[c] ──────────────────────────────────────────────── dry[c] (exact, gain 1) ──┐
mono = mean(in[c]) (non-finite → 0)                                              │
 └─► ClockDomain (host → f_int = 32768·scale(Tracking), linear interp, no AA) ─┐ │
       u = mono_int + g_fb·tanh(yP·gP + yS·gS)   [g_fb=0 unless Magic engaged] │ │
       yP = ShifterP(u, rP·chorusLFO)   yS = ShifterS(u, rS·chorusLFO')         │ │
       wet_int = gP·yP + gS·yS                                                  │ │
 ◄── ClockDomain (f_int → host, linear interp) ◄────────────────────────────────┘ │
 wet = ToneLPF(wet) ─────────────────────────────────────────────────────────► + ─► bypass xfade ─► out[c]
```
Feedback is taken post-level (Magic only acts when Primary/Secondary are up — C-006) and is soft-clipped (tanh) so
self-oscillation is bounded (T-013). Tone is outside the loop. rP = 2^(semitones/12); gains per D-005.

**D-003 Parameter table (frozen public interface; T-001, T-030).**
| # | key | name | range | default | type |
|---|---|---|---|---|---|
| 0 | `pitch` | Pitch | 0–1 | 0.5 | float |
| 1 | `pitch_exp` | Pitch Exp | 0–1 | 0.5 | float |
| 2 | `exp_enabled` | Exp Enabled | 0–1 | 0 | bool |
| 3 | `primary` | Primary | 0–1 | 0.7 | float |
| 4 | `secondary` | Secondary | 0–1 | 0.0 | float |
| 5 | `tracking` | Tracking | 0–1 | 0.8 | float |
| 6 | `tone` | Tone | 0–1 | 1.0 | float |
| 7 | `magic` | Magic | 0–1 | 0.3 | float |
| 8 | `magic_on` | Magic On | 0–1 | 0 | bool |
| 9 | `bypass` | Bypass | 0–1 | 0 | bool |
All ranges are linear (no skew) in the host; mappings to physical values happen in `refractor::mapping`.

**D-004 Pitch mapping.** Piecewise linear in semitones: knob 0 → −5 (a fourth below), 0.5 → 0, 1 → +4 (major third).
Constants `kPitchDownSemitones`, `kPitchUpSemitones` (A-013). Continuous: "every atonal pitch in between" (C-003).

**D-005 Level law.** Linear amplitude `gain = knob / 0.7`; max ≈ 1.43 (+3.1 dB boost above "2 o'clock", C-004, A-014).

**D-006 Secondary voice.** rS = 2·rP when pitch knob > 0.5 + ε, rS = rP/2 when < 0.5 − ε, ε = 0.01. Inside the
deadband rS = rP and the secondary uses a chorus LFO in quadrature (90°) with the primary's (C-007). Octave switch
across the deadband is crossfaded over 20 ms to avoid clicks.

**D-007 Tone.** One-pole low-pass on the wet signal; cutoff exponential from 1 kHz (knob 0) to 20 kHz (knob <1);
knob == 1 exactly → filter bypassed (C-005). Filter: `y += a·(x − y)`, `a = 1 − exp(−2π·fc/fs_host)`, fc smoothed in the log domain; bypass engages when target is +inf and smoothed fc ≥ 19 900 Hz.

**D-008 Magic.** Feedback gain `g_fb = magic · 1.15` when `magic_on`; ramps to 0 over 10 ms when disengaged.
Loop soft clip = `tanh`. Values with |x| < 1e-15 in any recursive state are flushed to 0 (T-024).

**D-009 Expression.** When `exp_enabled` ≥ 0.5, `pitch_exp` replaces `pitch` in D-004 (pedal: Pitch knob defeated, C-002).

**D-010 Pitch engine & Tracking.** Two-tap, triangular-crossfade delay-line shifter (spike S1, C-020..C-022) per voice,
running in a variable-rate internal clock domain: f_int = 32768 Hz × s, s = kClockScaleMin·(1/kClockScaleMin)^tracking
(exponential; 0.15 → 1.0). Window = 1024 internal samples, so wet lag = 512/f_int (15.6 ms at Tracking=1, 104 ms at 0),
and bandwidth/aliasing degrade as Tracking falls — the V1 clock-pot mechanism (C-010) and V2's "tighter tracking with
shorter delay" (C-009). Resampling uses linear interpolation with **no** anti-alias filter by design (C-012/C-013
"digital remnants"). Rejected alternatives: phase vocoder / Signalsmith-style spectral shifting (too clean, adds latency,
no natural "tracking lag" control); PSOLA (needs pitch detection, fails on chords — pedal is polyphonic).

**D-011 Calibration constants.** All tunable numbers live in `src/dsp/Calibration.hpp` with static_assert bounds.
They may change only at gate G-003, within bounds. Frozen tests reference them symbolically, so calibration within
bounds never invalidates a test.

**D-012 Chorus at noon.** Both voices always carry a sine detune LFO (depth 8 cents, 0.6 Hz), making noon a chorus (C-011). Effective ratio `r(t) = rVoice · 2^(kChorusDepthCents·sin(2π·kChorusRateHz·t + φ)/1200)`, t in internal-clock seconds, φ = 0 (primary) / π/2 (secondary), phase reset to 0 by `reset()`.

**D-013 Switching.** `bypass` and `magic_on` are plain bools for the host. The editor's two footswitch buttons use
`refractor::DualModeSwitch`: press toggles; release after ≥0.30 s reverts (momentary) (C-008). Bypass = 10 ms
equal-gain crossfade to exact dry; wet tails are cut (true-bypass behaviour). Name avoids the "Flexi-Switch" trademark.

**D-014 Real-time contract.** See `src/dsp/Engine.hpp` header: no alloc/lock/throw after `prepare`; per-sample smoothing
(one-pole, `a = 1 − exp(−1/(kSmoothingSeconds·fs))`, per host sample) so output is block-partition invariant; bypass crossfade is a linear ramp over round(kBypassFadeSeconds·fs) samples whose end state is exactly 0 or 1, after which the dry-only branch (`out = in`) or the full branch is taken without multiplication; `reset()` snaps smoothers; shifter phase 0 after reset;
non-finite input → 0; deterministic (no RNG, no time-dependence).

**D-015 Plugin wrapper.** `RefractorProcessor` owns one `Engine`, an APVTS built from `parameterSpecs()`
(`AudioParameterFloat`/`AudioParameterBool`, IDs = keys, version hint 1). `prepareToPlay` → `engine.prepare`, push all
current parameter values, `engine.reset()`. `processBlock` pushes parameter values once per block (engine smooths),
uses `juce::ScopedNoDenormals`, calls `engine.process` in place. Latency 0; tail = `Engine::kTailSeconds` (10 s).
`getBypassParameter()` returns the `bypass` parameter.

**D-016 State format.** APVTS `ValueTree` → XML via `copyXmlToBinary`. `setStateInformation`: `getXmlFromBinary`; accept
only if tag == APVTS state type; for each known key parse `value` with `String::getFloatValue`, reject non-finite, clamp
to range; unknown/missing keys → keep current value. Never throws; never allocates unboundedly (JUCE parser bounded by
blob size). Verified by T-031, T-032.

**D-017 Bus layouts.** Supported: mono→mono, mono→stereo, stereo→stereo. Rejected: everything else. Default: stereo→stereo.

**D-018 UI.** Original design: dark panel, seven rotary knobs (Pitch, Primary, Secondary, Tracking, Tone, Magic, Pitch Exp
+ Exp toggle), two footswitch buttons (Active, Magic) with LEDs. Font: JUCE default sans. Resizable 1×–2×, base 640×360.
No imagery of the pedal, no pedal names. `SliderAttachment`/`ButtonAttachment` for all controls.

**D-019 Security posture.** Threat model A-010. Controls: T-023 (audio), T-032 (state), T-051 (no I/O), T-050 (pins),
T-052 (ASan+UBSan run of dsp/rt tests on Linux in CI). No secrets in the repository; CI uses only `GITHUB_TOKEN`.

**D-020 CI.** `.github/workflows/ci.yml` with jobs `build-test` (matrix: `ubuntu-24.04`, `macos-15`, `windows-2022`;
Release; runs ctest excluding label `perf`; runs pluginval), `sanitizers` (ubuntu-24.04, Debug, REFRACTOR_SANITIZE=ON,
plugin OFF, runs dsp/rt tests), `perf` (ubuntu-24.04, Release, `ctest -L perf`), `freeze` (T-070). All `uses:` pinned
by 40-hex SHA (T-050). Artifacts: VST3 bundle + Standalone per OS, retention 14 days.

**D-021 Profile boundary.** DSP numbers (lags, ratios) are tested as behavioural requirements with tolerances derived
from spike S1, not published as results; hence no `computational` profile.

## Interfaces
Public C++ interfaces are committed as stubs: `src/dsp/{Params,Mapping,PitchShifter,ClockDomain,DualModeSwitch,Engine}.hpp`,
`src/plugin/{PluginProcessor,PluginEditor}.h`. Stubs throw `refractor::NotImplemented`. The implementer may add
private members and new private files, but must not change any public signature, the parameter table, or file paths
referenced by tests.

## Decision rules (if → then)

| ID | If | Then |
|---|---|---|
| DR-01 | A dependency fetch fails (network) | Retry 3× with 30 s backoff; then halt with BLOCKED.md (never unpin). |
| DR-02 | A frozen test fails and the implementation seems right | Re-read D-### and the test's docstring; add diagnostics in a scratch test (not committed to tests/); if the test is provably wrong → TEST_CHALLENGE.md and halt. Never edit frozen files. |
| DR-03 | Pitch-ratio tests (T-006/T-009/T-017/T-019) fail by < 2× tolerance | Check resampler phase continuity and that the chorus LFO is zero-mean; verify with `research/spikes/S1-delayline-shifter`; do not change tolerances. |
| DR-04 | T-011 trails not monotonic | Check feedback is inside the internal clock domain and taken post-shifter; check `tanh` placement; calibrate only within G-003 bounds is NOT allowed before G-003 — fix the implementation. |
| DR-05 | Performance T-060/T-061 misses target | Profile with `perf` on Linux; allowed optimisations: SIMD-free loop tightening, precomputed crossfade tables, per-block (not per-sample) coefficient computation **only** if T-021 still passes. If still failing after 2 attempts → halt at G-003 with numbers and a proposal. |
| DR-06 | pluginval fails at strictness 10 | Read `pluginval-logs`; fix the plugin. If a failure is a pluginval defect (reproducible with JUCE's AudioPluginDemo at the same level) → document in DEVIATIONS.md with evidence and continue; never lower the level. |
| DR-07 | A CI runner image label is unavailable | Use the nearest newer GA label of the same OS family from actions/runner-images README; log in DEVIATIONS.md. |
| DR-08 | Security scan / T-032 / T-023 / sanitizer failure | Fix before any other step; these are Critical. |
| DR-09 | macOS build fails on code signing | CI builds unsigned (ad-hoc) — set `CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="-"`; signing/notarisation is out of scope. |
| DR-11 | On Windows, ctest's `bash` resolves to WSL (`C:\Windows\System32\bash.exe`) | CI Windows job prepends `C:\Program Files\Git\bin` to PATH before ctest (R-015). |
| DR-10 | A step needs a decision not covered here | Default rule below. |

**Default rule:** Choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with rationale,
and continue — **unless** it touches frozen tests, security, data integrity, a public interface, or research integrity,
in which case halt and write `BLOCKED.md`.

## Immutability
Frozen files (tests/, bench/, `tests/FROZEN_MANIFEST.sha256` entries) may not be modified, skipped, marked expected-fail
or weakened. `Calibration.hpp` is not frozen but may change only at G-003 (D-011).
