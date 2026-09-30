<!-- STATUS HEADER (Phase 5) -->
**STATUS: IMPLEMENTATION IN PROGRESS (S-001…S-008 done, S-009 next)** — generated 2026-09-28 by planning-protocol v3.1; frozen suite amended 2026-09-30 with owner approval (plan/amendments/AMENDMENTS.md).
Profiles: `software` (deploys=false). Research: 3 rounds (saturated). Pre-mortem: 2 rounds (converged, 0 Critical/High open).
Freeze: 17 files (tests/FROZEN_MANIFEST.sha256, re-hashed after the amendments), red-verified (research/spikes/S2-env/red-verification.md; guard tests T-051/T-053/T-070 mutation-verified, D-022). Gates: G-003 (listening/IP, required), G-002 (guard only).
Deviations from intake defaults: D-001 (JUCE 8.0.15 → licence AGPL-3.0-only instead of GPLv3); D-010 revised (phase-vocoder shifter).

# Refractor — implementation plan

Goal: a VST3 + Standalone plugin reproducing the *behaviour* of the V2 polyphonic pitch-warping pedal described in
`research/claims.json` (C-001..C-016), built with JUCE 8.0.15, tested by the frozen suite in `tests/`.
Read `HANDOFF.md` first. Branch for all work: `impl/refractor-v0.1` (created in S-001). Never commit to the generation branch.

Conventions for every step: `$REPO` = repository root; `$B` = `$REPO/build` (Release) ; `$FC` = `$REPO/.fc` (FetchContent
cache). "Run tests X" means `ctest --test-dir $B -R '^(dsp_tests|rt_tests|plugin_tests)$' --output-on-failure` and
filtering Catch2 cases with `$B/tests/refractor_dsp_tests "[T-00X]"`. A step's checkpoint is recorded by appending one
line to `.impl/progress.log`: `S-### done <ISO-8601 UTC> <git sha>` and committing it.

### S-001 Bootstrap implementation branch
- Tier: Haiku
- Profile: software
- Depends on: none
- Inputs: generation branch `gen-20260928T151837Z-refractor-rainbow-machine-vst`; env `GH_TOKEN` (A-016)
- Actions:
  1. `git clone https://github.com/qualitycoding/refractor $REPO && cd $REPO && git checkout gen-20260928T151837Z-refractor-rainbow-machine-vst`
  2. `python3 tests/scripts/check_freeze.py .` — must print `T-070 OK`; else halt BLOCKED.md ("freeze manifest mismatch on checkout").
  3. `git checkout -b impl/refractor-v0.1`
  4. `curl -fsSL https://www.gnu.org/licenses/agpl-3.0.txt -o LICENSE` (fallback if unreachable: `curl -fsSL https://raw.githubusercontent.com/spdx/license-list-data/main/text/AGPL-3.0-only.txt -o LICENSE`).
  5. Create `.gitattributes` with lines `* text=auto eol=lf` and `*.png binary` and `*.wav binary`.
  6. Create `.gitignore` with `build*/`, `.fc/`, `pluginval-logs/`, `*.user`.
  7. Create `README.md`: name, one-paragraph description (no third-party product names — A-003), build commands from HANDOFF.md, licence AGPL-3.0-only, AI-assistance note (A-017).
  8. `mkdir -p .impl && touch .impl/progress.log DEVIATIONS.md`
  9. `git add -A && git commit -m "S-001 bootstrap" && git push -u origin impl/refractor-v0.1`
- Outputs: LICENSE, .gitattributes, .gitignore, README.md, DEVIATIONS.md, .impl/progress.log
- Evidence produced: T-070 pass (freeze intact)
- Done when: `python3 tests/scripts/check_freeze.py .` prints `T-070 OK`; `python3 tests/scripts/check_ip.py .` prints `T-053 OK`
- Checkpoint: `S-001 done`
- On failure: network → DR-01; freeze mismatch → halt BLOCKED.md.
- Gate: none
- Relevant decisions/claims: A-002, A-003, A-017, D-001

### S-002 Environment and red baseline
- Tier: Haiku
- Profile: software
- Depends on: S-001
- Inputs: plan/ENVIRONMENT.md
- Actions:
  1. Execute plan/ENVIRONMENT.md §Linux setup exactly.
  2. `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_BASE_DIR=$PWD/.fc`
  3. `cmake --build build -j$(nproc)`
  4. `ctest --test-dir build -E '^(pluginval|perf)$' --output-on-failure > .impl/red-baseline.txt 2>&1 || true`
  5. Verify every Catch2 failure in `.impl/red-baseline.txt` is "due to unexpected exception" with a NotImplemented message, and `check_pins` fails only with ".github/workflows missing".
- Outputs: build/, .impl/red-baseline.txt
- Evidence produced: none (baseline)
- Done when: build exits 0; `grep -c "due to unexpected exception" .impl/red-baseline.txt` ≥ 42 and no line contains `error:` from the compiler.
- Checkpoint: `S-002 done`
- On failure: compiler error in frozen tests → TEST_CHALLENGE.md, halt; missing apt package → install it, log in DEVIATIONS.md.
- Gate: none
- Relevant decisions/claims: C-034, C-038, D-001

### S-003 Parameter table and mappings
- Tier: Sonnet
- Profile: software
- Depends on: S-002
- Inputs: src/dsp/Params.{hpp,cpp}, src/dsp/Mapping.{hpp,cpp}, src/dsp/Calibration.hpp, D-003..D-009
- Actions:
  1. Implement `parameterSpecs()` as a `static constexpr std::array<ParamSpec,10>` with exactly the D-003 table; `spec(id)` returns `parameterSpecs()[size_t(id)]`.
  2. Implement mappings per `Mapping.hpp` comments and D-004/D-005/D-007/D-008: clamp to [0,1]; NaN → parameter default (pitch 0.5, level 0.7, tracking 0.8, tone 1, magic 0.3); `toneToCutoffHz(k)` = `kToneMinCutoffHz * pow(20000/kToneMinCutoffHz, k)` for k<1, `+inf` at k==1.
  3. Build; run `build/tests/refractor_dsp_tests "[T-001],[T-002],[T-003],[T-004],[T-005]"`.
- Outputs: src/dsp/Params.cpp, src/dsp/Mapping.cpp
- Evidence produced: T-001, T-002, T-003, T-004, T-005
- Done when: T-001, T-002, T-003, T-004, T-005 pass.
- Checkpoint: `S-003 done`
- On failure: DR-02.
- Gate: none
- Relevant decisions/claims: D-003..D-009, C-003, C-004, C-005, A-013, A-014

### S-004 Dual-mode switch
- Tier: Sonnet
- Profile: software
- Depends on: S-002
- Inputs: src/dsp/DualModeSwitch.{hpp,cpp}, D-013
- Actions:
  1. Implement: `press(t)`: if not down: `down=true; pressTime=t; beforePress=engaged; engaged=!engaged`. `release(t)`: if down: `down=false; if (t-pressTime >= kHoldSeconds) engaged=beforePress`. `set(e)`: `engaged=e; down=false`.
  2. Run `build/tests/refractor_dsp_tests "[T-036]"`.
- Outputs: src/dsp/DualModeSwitch.cpp
- Evidence produced: T-036
- Done when: T-036 passes.
- Checkpoint: `S-004 done`
- On failure: DR-02.
- Gate: none
- Relevant decisions/claims: D-013, C-008

### S-005 Shifter and clock domain
- Tier: Opus
- Profile: software
- Depends on: S-003
- Inputs: src/dsp/PitchShifter.*, src/dsp/ClockDomain.*, research/spikes/S4-phase-vocoder/, D-010 (rev. 2)
- Actions:
  1. Implement `PhaseVocoderShifter` per D-010 (FFT size `kWindowSamples`, overlap `kOverlap`; peak picking, valley-split regions, phase-locked region shift by the rounded frequency difference, alias folding, Hann/Hann overlap-add with scale 8/(3·overlap); latency = N). No allocation after `prepare`.
  2. Implement `ClockDomain` in `ClockDomain.ipp`: per host sample advance the tick accumulator by `f_int/hostRate`; at each tick feed the area-average of the piecewise-linear host signal since the previous tick; host output = causal linear interpolation of the last two internal outputs. Scale smoothing: one-pole, `kSmoothingSeconds`, per host sample.
  3. Validate in isolation with `research/spikes/S4-phase-vocoder/pv_sweep.cpp` (pitch error < 0.01 %), then through Engine in S-006.
- Outputs: src/dsp/PitchShifter.{hpp,cpp}, src/dsp/ClockDomain.{cpp,ipp}
- Evidence produced: spike S4 outputs (C-023, C-024)
- Done when: `cmake --build build` succeeds; `grep -c NotImplemented` prints 0 for both files; `pv_sweep` worst error < 0.01 %.
- Checkpoint: `S-005 done`
- On failure: DR-03.
- Gate: none
- Relevant decisions/claims: D-010, C-022, C-023, C-024

### S-006 Engine: voices, chorus, tone, bypass, smoothing, channels
- Tier: Opus
- Profile: software
- Depends on: S-004, S-005
- Inputs: src/dsp/Engine.{hpp,cpp}, D-002, D-004..D-007, D-009, D-012..D-014, D-017
- Actions:
  1. Implement `Engine::Impl` per D-002 with Magic feedback gain fixed at 0 for now: two phase-vocoder shifters in one ClockDomain callback, chorus LFOs (primary phase 0, secondary +90°), secondary octave logic with 20 ms crossfade, one-pole tone LPF at host rate (bypassed when target cutoff is +inf and smoothed cutoff ≥ 19.9 kHz), bypass crossfade, per-sample one-pole smoothers (kSmoothingSeconds) for pitch semitones, gains, clock scale, cutoff (log domain), magic feedback.
  2. Channel rules from Engine.hpp header; sanitise non-finite input to 0 before any use.
  3. `nominalWetLagSeconds()` = `(kWindowSamples/2) / (kNominalClockHz * trackingToClockScale(targetTracking))`.
  4. Run `build/tests/refractor_dsp_tests "[T-006],[T-007],[T-008],[T-009],[T-010],[T-014],[T-015],[T-016],[T-017],[T-018],[T-019],[T-020],[T-021],[T-025],[T-026]"`.
- Outputs: src/dsp/Engine.cpp
- Evidence produced: T-006, T-007, T-008, T-009, T-010, T-014, T-015, T-016, T-017, T-018, T-019, T-020, T-021
- Done when: all listed tests pass.
- Checkpoint: `S-006 done`
- On failure: DR-02, DR-03.
- Gate: none
- Relevant decisions/claims: D-002, D-004..D-007, D-009, D-010, D-012..D-014, D-017, C-003, C-007, C-009, C-011

### S-007 Magic loop and robustness
- Tier: Opus
- Profile: software
- Depends on: S-006
- Inputs: src/dsp/Engine.cpp, D-008, D-014
- Actions:
  1. Enable feedback: `u = x_int + g_fb * tanh(gP*yP + gS*yS)` inside the internal-clock callback, using the previous internal tick's voice outputs; `g_fb` smoothed, 10 ms ramp to 0 on disengage; return `softCeiling(w)` (kWetCeiling, D-008) while the loop tap uses the un-limited wet.
  2. Flush |x| < kFlushThreshold to 0 in shifter buffers writes, filter state, smoothers' outputs of zero targets.
  3. Run `build/tests/refractor_dsp_tests` (all) and `build/tests/refractor_rt_tests`.
- Outputs: src/dsp/Engine.cpp (and shifter if flushing needs it)
- Evidence produced: T-011, T-012, T-013, T-022, T-023, T-024 (+ all of S-006 still green)
- Done when: `ctest --test-dir build -R '^(dsp_tests|rt_tests)$'` passes.
- Checkpoint: `S-007 done`
- On failure: DR-04, DR-08.
- Gate: none
- Relevant decisions/claims: D-008, D-014, C-006, C-015, C-021, C-025

### S-008 Performance
- Tier: Sonnet
- Profile: software
- Depends on: S-007
- Inputs: bench/, tests/scripts/check_perf.py
- Actions:
  1. `ctest --test-dir build -L perf --output-on-failure` locally (informational; authoritative run is CI, S-010).
  2. If median > target, apply DR-05.
- Outputs: possibly src/dsp/*.cpp
- Evidence produced: T-060, T-061 (local)
- Done when: local `perf` test passes, or DR-05 invoked and logged.
- Checkpoint: `S-008 done`
- On failure: DR-05.
- Gate: none
- Relevant decisions/claims: A-011, R-008

### S-009 Plugin processor and state
- Tier: Sonnet
- Profile: software
- Depends on: S-007
- Inputs: src/plugin/PluginProcessor.{h,cpp}, D-015, D-016, D-017
- Actions:
  1. Implement per D-015/D-016/D-017. APVTS state type identifier `"REFRACTOR"`; bool params via `AudioParameterBool`; `ParameterID{key, 1}`.
  2. `setStateInformation`: implement D-016 manually (do NOT use `apvts.replaceState` on unvalidated trees). Null/zero-size input → no-op.
  3. Run `build/tests/refractor_plugin_tests`.
- Outputs: src/plugin/PluginProcessor.h (private members only), src/plugin/PluginProcessor.cpp
- Evidence produced: T-030, T-031, T-032, T-033, T-034, T-035
- Done when: `ctest --test-dir build -R '^plugin_tests$'` passes.
- Checkpoint: `S-009 done`
- On failure: DR-02, DR-08.
- Gate: none
- Relevant decisions/claims: D-015, D-016, D-017, A-010

### S-010 CI pipeline
- Tier: Sonnet
- Profile: software
- Depends on: S-009
- Inputs: D-020, plan/ENVIRONMENT.md §CI
- Actions:
  1. Create `.github/workflows/ci.yml` implementing D-020 with the action SHAs from ENVIRONMENT.md §CI (checkout, setup-python, upload-artifact). Trigger: push and pull_request on `impl/**` and `main`.
  2. Linux job: install §Linux packages + `xvfb`; run `xvfb-run -a ctest --test-dir build -E '^perf$' --output-on-failure` with `PLUGINVAL=$RUNNER_TEMP/pluginval` (downloaded and sha256-checked per ENVIRONMENT.md).
  3. macOS/Windows jobs: same ctest command without xvfb (Windows: VS generator and `-C Release` per ENVIRONMENT.md, and prepend `C:\Program Files\Git\bin` to PATH — DR-11); pluginval from the matching v1.0.4 zip, verified against the sha256 in plan/ENVIRONMENT.md §Pinned artefacts (mismatch → fail the job); macOS configure adds `-DCMAKE_OSX_ARCHITECTURES=arm64` (DR-09 for signing).
  4. `python3 tests/scripts/check_pins.py .` locally → `T-050 OK`.
  5. Push; poll run status: `curl -s -H "Authorization: Bearer $GH_TOKEN" "https://api.github.com/repos/qualitycoding/refractor/actions/runs?branch=impl/refractor-v0.1&per_page=1"` until `status == completed`.
- Outputs: .github/workflows/ci.yml
- Evidence produced: T-040 (3 OS), T-050, T-052, T-060, T-061 (CI), T-070 (CI)
- Done when: latest run on `impl/refractor-v0.1` has `conclusion == success` for every job.
- Checkpoint: `S-010 done <run url>`
- On failure: DR-06, DR-07, DR-09, DR-05; platform build break inside JUCE → D-001 rule.
- Gate: none
- Relevant decisions/claims: D-020, C-033, C-037, D-019

### S-011 Editor UI
- Tier: Sonnet
- Profile: software
- Depends on: S-009
- Inputs: src/plugin/PluginEditor.{h,cpp}, D-018, D-013
- Actions:
  1. Implement D-018 layout; footswitch buttons drive `bypass`/`magic_on` through a `DualModeSwitch` each (mouseDown → press, mouseUp → release, then set the parameter via `setValueNotifyingHost` inside `beginChangeGesture/endChangeGesture`); LEDs reflect parameter state via attachment listeners.
  2. `setResizable(true, true)`; `setResizeLimits(640, 360, 1280, 720)`; fixed aspect ratio 16:9.
  3. `python3 tests/scripts/check_ip.py .` and push; CI (with GUI tests on Linux via xvfb) must stay green.
- Outputs: src/plugin/PluginEditor.{h,cpp}
- Evidence produced: T-040 GUI portion, T-053
- Done when: CI green on the pushed commit; `check_ip` prints `T-053 OK`.
- Checkpoint: `S-011 done <run url>`
- On failure: DR-06.
- Gate: none
- Relevant decisions/claims: D-018, D-013, A-003

### S-012 Demo renderer and editor snapshot tools
- Tier: Sonnet
- Profile: software
- Depends on: S-011
- Inputs: G-003 evidence spec
- Actions:
  1. Add CMake option `REFRACTOR_BUILD_TOOLS` (default OFF) and `tools/render_demos.cpp` (links refractor_dsp; synthesises inputs deterministically — Karplus-Strong phrase, three-note chord, bass line — writes 48 kHz 24-bit WAVs with a minimal RIFF writer; renders the eight presets named in plan/GATES.md; writes `demo/PRESETS.md` listing parameter values).
  2. Add `tools/snapshot_editor.cpp` (links refractor_plugin_shared; creates processor+editor at 640×360 under `ScopedJuceInitialiser_GUI`, `createComponentSnapshot`, saves `demo/editor.png` via `PNGImageFormat`). On Linux run under `xvfb-run -a`.
  3. `cmake -S . -B build -DREFRACTOR_BUILD_TOOLS=ON && cmake --build build && build/tools/render_demos demo && xvfb-run -a build/tools/snapshot_editor demo/editor.png`
- Outputs: tools/*.cpp, CMakeLists.txt (tools option only), demo/**
- Evidence produced: G-003 bundle inputs
- Done when: `ls demo/*.wav | wc -l` ≥ 8 and `demo/editor.png` exists; T-051 still passes (tools are outside `src/`).
- Checkpoint: `S-012 done`
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: G-003, A-005

### S-013 Pre-gate verification and listening gate
- Tier: Opus
- Profile: software
- Depends on: S-010, S-012
- Inputs: all
- Actions:
  1. Confirm CI green on HEAD; `python3 tests/scripts/check_freeze.py .`; `python3 tests/scripts/check_no_io.py .`; `python3 tests/scripts/check_ip.py .`.
  2. Write `GATE-G-003.md` with the bundle defined in plan/GATES.md; commit; push; halt.
  3. On `calibrate:` response → edit only `src/dsp/Calibration.hpp`, rebuild, run full ctest, push, wait for CI green, re-render demos, re-open gate (max 3 rounds).
- Outputs: GATE-G-003.md, possibly src/dsp/Calibration.hpp
- Evidence produced: SC-10 sign-off; T-051, T-053, T-070 re-verified
- Done when: human replies `proceed`.
- Checkpoint: `S-013 gate-open` then `S-013 done <response>`
- On failure: static_assert failure on calibration → reply to human with bounds, re-open gate.
- Gate: G-003
- Relevant decisions/claims: D-011, R-001, R-002

### S-014 Completion
- Tier: Haiku
- Profile: software
- Depends on: S-013
- Inputs: all
- Actions:
  1. `python3 tests/scripts/check_no_stubs.py .` → `T-071 OK`.
  2. Update README (usage, parameters table from D-003); add `CHANGELOG.md` entry 0.1.0 (unreleased).
  3. Push; wait for CI green; open a pull request `impl/refractor-v0.1` → `main` via `curl -X POST -H "Authorization: Bearer $GH_TOKEN" https://api.github.com/repos/qualitycoding/refractor/pulls -d '{"title":"Refractor 0.1.0","head":"impl/refractor-v0.1","base":"main"}'` (create `main` from the generation branch first if absent: `git push origin gen-20260928T151837Z-refractor-rainbow-machine-vst:refs/heads/main`). Do not merge; do not tag; do not release (G-002).
- Outputs: README.md, CHANGELOG.md, PR
- Evidence produced: T-071, full-suite CI pass
- Done when: every test in plan/TRACEABILITY.md passes in the latest CI run; PR open.
- Checkpoint: `S-014 done <pr url>`
- On failure: default rule; any release request → G-002.
- Gate: none (G-002 only if a release is requested)
- Relevant decisions/claims: A-011, G-002
