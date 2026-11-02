# Refractor

A polyphonic pitch-warping effect plugin (VST3 and Standalone) for guitar, bass, keys and voice. It shifts the
input from a fourth below to a major third above, adds an octave voice, and has a regeneration control
("Magic") that feeds the shifted voices back on themselves for aliasing, pitch-bending trails and
self-oscillation. A "Tracking" control sets the lag between dry and wet signal by changing the effect's
internal clock rate.

It is a behavioural reproduction of a classic pitch-warping pedal from published control descriptions. It uses no
third-party artwork, names or trade dress, and a test (T-053) checks that those names do not appear in the source.

> **Status: implemented, but not signed off by ear.** This default branch
> (`gen-20260928T151837Z-refractor-rainbow-machine-vst`) holds the plan, the public DSP and plugin interfaces
> (`src/`) and the frozen tests (`tests/`). The finished code is on the **`impl/refractor-v0.1`** branch, which has
> not been merged.

## What the implementation branch has

- C++20 on JUCE 8.0.15, with the pitch-warping DSP, a plugin with an editor (16:9, resizable), a benchmark, demo
  renders and a perf check.
- CI that passed on all 6 jobs at the last recorded run (`cd87d68`): build and tests on Ubuntu, macOS and Windows,
  pluginval at strictness 10, ASan and UBSan, performance tests, and the freeze and policy checks.
- Nine demo presets rendered from synthetic plucked-string input, listed in `demo/PRESETS.md`.
- **Not verified:** how close it sounds to the pedal. The pedal's DSP is unpublished and there are no reference
  recordings, so fidelity has to be judged by ear. The owner sign-off (gate G-003) is open, and the blind listening
  test is blocked until a reference and candidate bundle exists.
- Known design limit: the lag is 31 ms to 208 ms, and chord partials are clean only below about Tracking 0.6 (a G
  triad is 42 % clean at Tracking 1).

## Build (from `impl/refractor-v0.1`)

```bash
git checkout impl/refractor-v0.1
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_BASE_DIR=$PWD/.fc
cmake --build build -j"$(nproc)"
xvfb-run -a ctest --test-dir build -E '^perf$' --output-on-failure
```

See `plan/ENVIRONMENT.md` for pinned tool versions and OS-specific setup. `python3 tests/scripts/check_freeze.py .`
checks that the frozen tests are unchanged.

## Licence

AGPL-3.0-only (JUCE 8 open-source option). See `LICENSE` on the implementation branch.

The plan and the code were produced with AI assistance (Claude).
