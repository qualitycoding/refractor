# GATE G-003 — Listening & IP sign-off  (OPEN — awaiting owner response)

## 1. CI evidence
Run on `cd87d68` (all src/ and tests/ content of the gate build) — https://github.com/qualitycoding/refractor/actions/runs/36765851852 — **all 6 jobs green**:
build-test on ubuntu-24.04, macos-15 (arm64, unsigned) and windows-2022 (each runs dsp/rt/plugin tests, pluginval 1.0.4 strictness 10, policy checks);
sanitizers (ASan+UBSan: dsp_tests, rt_tests); perf (T-060/T-061); freeze and policy checks.
Artifacts (14-day retention): [linux](https://github.com/qualitycoding/refractor/actions/runs/36765851852/artifacts/11120782380) ·
[macos](https://github.com/qualitycoding/refractor/actions/runs/36765851852/artifacts/11120224531) · [windows](https://github.com/qualitycoding/refractor/actions/runs/36765851852/artifacts/11120469494).
The earlier run on `835a6f3` was also green on all three OSes. Commits after `cd87d68` change only documentation, `demo/LISTENING.md` and the optional `tools/` (not built in CI).

## 2. Demo WAVs (`demo/`, 48 kHz 24-bit mono; `demo/PRESETS.md` lists every setting)
Nine presets (the eight required plus a low-note bass case, because low notes are where a short window hurts most). Inputs are
synthesised Karplus-Strong plucks in `demo/input/`. **Preset 08 peaks at +11.4 dBFS, so `demo/08_…wav` is hard-clipped** (the plugin output is not); the listening set ships it pre-trimmed by −12 dB (`render_demos <dir> -12`).

## 3. Calibration values (src/dsp/Calibration.hpp) and bounds
| constant | value | bounds |
|---|---|---|
| kWindowSamples (FFT size = lag in internal samples) | 1024 | 512…4096, power of two |
| kOverlap | 4 | 4…8 |
| kClockScaleMin / Max | 0.15 / 1.0 | 0.10…0.50 / ==1.0 |
| kWetCeiling | 4.0 (+12 dBFS) | 2.0…8.0 |
| kChorusDepthCents / Rate | 8 cents / 0.6 Hz | 2…20 cents |
| kMagicMaxFeedback | 1.15 | 0.9…1.5 |
| kToneMinCutoffHz | 1000 | 500…3000 |
| kPitchDown / Up, kUnityKnob | −5 st / +4 st / 0.7 | (no bounds; C-003, A-013, A-014) |
Consequences of kWindowSamples=1024: lag 31 ms (Tracking 1) … 208 ms (Tracking 0); chord partials are clean when bin width f_int/N ≲ 16 Hz, i.e. below about Tracking 0.6; at Tracking 1 a G triad is 42 % clean (C-024). **Lag × resolution = 1, so chord clarity depends only on lag, not on N.** `kWindowSamples` just scales the lag range (512: 16–104 ms; 1024: 31–208 ms; 2048: 62–417 ms); bandwidth depends on Tracking only. See demo/LISTENING.md.

## 4. Editor and IP
`demo/editor.png` (640×360 default; resizable 16:9 up to 1280×720). T-053 output: `T-053 OK` (no "Rainbow Machine", "EarthQuaker", "Flexi-Switch" anywhere in src/, README, plugin strings). Footswitch class is `DualModeSwitch`.

## Questions for the owner
a. Character versus your expectation of the pedal, per preset: ok / too clean / too dirty / wrong lag / wrong pitch. Note there are no reference captures: the pedal's DSP is unpublished, so fidelity is judged by ear.
b. Calibration: is 31–208 ms the right lag range, and are chords clear enough at high Tracking (a lag-range choice: `kWindowSamples`, `kClockScaleMin`)? Is +12 dBFS wet ceiling acceptable at maximum Magic (preset 08 is +11.4 dBFS), or should `kWetCeiling` go down (minimum allowed 2.0 = +6 dBFS)?
c. Is the UI free of anything resembling the original pedal's trade dress?
Allowed responses: `proceed` | `calibrate: CONST=value[, …]` | `proceed-with-rescope: …` | `stop`.

## Known limits of the evidence
1. macOS/Windows plugin *behaviour in a DAW* is verified only by CI (pluginval + tests), not by ear or in a host.
2. Fresh-context reviews were role-switched passes by the same model, not independent reviewers.
3. Listening set: demo/LISTENING.md explains what each file is for (also `refractor-listening-set.zip` with A/B and sweep files).
