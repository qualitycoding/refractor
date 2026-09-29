# Refractor

Refractor is a polyphonic pitch-warping effect plugin (VST3 and Standalone) for guitar, bass, keys and voice. It shifts
the input from a fourth below to a major third above, adds an octave voice, and has a regeneration control ("Magic")
that feeds the shifted voices back on themselves for aliasing, pitch-bending trails and self-oscillation. A "Tracking"
control sets the lag between the dry and wet signals by changing the effect's internal clock rate.

It is a behavioural reproduction from published control descriptions, inspired by a classic FV-1-era pitch-warping
pedal. It contains no third-party artwork, names or trade dress.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_BASE_DIR=$PWD/.fc
cmake --build build -j"$(nproc)"
ctest --test-dir build -E '^perf$' --output-on-failure
```

See `plan/ENVIRONMENT.md` for pinned tool versions and OS-specific setup.

## Licence

AGPL-3.0-only (JUCE 8 open-source option). See `LICENSE`.

## AI assistance

The plan and the code in this repository were produced with AI assistance (Claude).
