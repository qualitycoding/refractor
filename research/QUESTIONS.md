# Question tree (profile: software) — status after round 3

## Engineering
- Q1 What does the target pedal do, control by control? → D-002..D-013, T-001..T-017
  - Q1.1 Control list & ranges (V2)? → C-001, C-003, C-004, C-005 — **answered**
  - Q1.2 Secondary semantics in V2 vs V1? → C-007 — **answered (V2 only)**
  - Q1.3 Magic semantics? → C-006, C-015 — **answered**
  - Q1.4 Tracking semantics / mechanism? → C-009, C-010 — **answered (behaviour T1; mechanism T4)**
  - Q1.5 Switch behaviour (latch/momentary)? → C-008 — **answered**
  - Q1.6 Expression input? → C-002 — **answered**
  - Q1.7 Internal architecture (DSP chip, algorithm)? → C-012, C-013, C-014, C-017 — **partially** (V1 FV-1 ROM, V2 unknown) → R-001, R-002
- Q2 Which pitch-shift algorithm reproduces "tracking lag", polyphony and aliasing? → D-010 — **answered by spikes S3/S4** (S1's delay-line answer was wrong at other input frequencies: E-001; phase vocoder verified: C-022..C-025)
- Q3 Framework/version/licence that builds on all three CI OSes? → D-001 — **answered** (C-030, C-031, C-033, C-035)
- Q4 Linux build deps & CMake version verified in sandbox? → ENVIRONMENT.md — **answered** (C-034, C-038)
- Q5 Validation tooling (pluginval flags; Catch2 pin)? → T-040, T-050 — **answered** (C-036, C-037)
- Q6 Threat surface of a plugin (state blobs, audio, parameters)? → D-016, D-019, T-023, T-032 — **answered (design)**
- Q7 Performance headroom of two phase-vocoder voices + resampling? → T-060 — **answered** (worst case 2.3 % of a core at 48 kHz, S4)
- Q8 Trademark exposure of names? → A-003, T-053 — **answered** ("Rainbow Machine" ®, "Flexi-Switch" ® on EQD page, C-001/C-008)

Pruned: Signalsmith Stretch licensing (rejected by D-010); AU/AAX SDK questions (out of scope A-008).
