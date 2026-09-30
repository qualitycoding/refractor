# GATE G-003 — Listening & IP sign-off  (DRAFT — not yet openable, see "Open evidence gaps")

## 1. CI evidence
| item | status |
|---|---|
| Run on `835a6f3` — https://github.com/qualitycoding/refractor/actions/runs/36763498537 | Linux ✔, macOS-15 arm64 ✔, sanitizers (ASan+UBSan) ✔, perf ✔, freeze/policy ✔ at last poll; **Windows job was still running** |
| Artifacts (VST3/Standalone/pluginval logs) | `refractor-linux`, `refractor-macos`, `refractor-windows` on that run page (14-day retention) |
| Commits made after `835a6f3` (editor fixes, tools, demos, this file) | **not pushed, not run in CI** (token revoked) |

## 2. Demo WAVs (`demo/`, 48 kHz 24-bit mono; `demo/PRESETS.md` lists every setting)
Nine presets (the eight required plus a low-note bass case, because low notes are where a short window hurts most). Inputs are
synthesised Karplus-Strong plucks in `demo/input/`. **Preset 08 peaks at +11.4 dBFS, so its WAV is hard-clipped** (the plugin output is not).

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
Consequences of kWindowSamples=1024: lag 31 ms (Tracking 1) … 208 ms (Tracking 0); chord partials are clean when bin width f_int/N ≲ 16 Hz, i.e. below about Tracking 0.6; at Tracking 1 a G triad is 42 % clean (C-024). 2048 doubles lag (62–417 ms) and halves the smear.

## 4. Editor and IP
`demo/editor.png` (640×360 default; resizable 16:9 up to 1280×720). T-053 output: `T-053 OK` (no "Rainbow Machine", "EarthQuaker", "Flexi-Switch" anywhere in src/, README, plugin strings). Footswitch class is `DualModeSwitch`.

## Questions for the owner
a. Character versus your expectation of the pedal, per preset: ok / too clean / too dirty / wrong lag / wrong pitch. Note there are no reference captures: the pedal's DSP is unpublished, so fidelity is judged by ear.
b. Calibration: is 31–208 ms the right lag range, and are chords clear enough at high Tracking (`kWindowSamples` 1024 vs 2048)? Is +12 dBFS wet ceiling acceptable at maximum Magic (preset 08 is +11.4 dBFS), or should `kWetCeiling` go down (minimum allowed 2.0 = +6 dBFS)?
c. Is the UI free of anything resembling the original pedal's trade dress?
Allowed responses: `proceed` | `calibrate: CONST=value[, …]` | `proceed-with-rescope: …` | `stop`.

## Open evidence gaps (why this draft cannot be opened yet)
1. Windows result for `835a6f3` unknown; macOS/Windows plugin behaviour is verified only by CI.
2. HEAD has never run in CI. 3. pluginval on macOS/Windows is exercised only in CI. 4. Fresh-context reviews were role-switched passes by the same model, not independent reviewers.
