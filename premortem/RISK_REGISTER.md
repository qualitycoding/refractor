# Risk register (after pre-mortem round 2)

| ID | Risk | Sev | Likelihood | Mitigation in plan | Residual |
|---|---|---|---|---|---|
| R-001 | Behavioural model sounds unlike the real V2 (architecture undocumented, C-017; no reference captures, A-005) | High | Medium | Calibration constants + G-003 listening loop (max 3 rounds); tests are calibration-agnostic (D-011) | Medium — subjective; may need owner captures later |
| R-002 | V1 mechanism claims (FV-1 ROM, clock-pot Tracking) are hobbyist-sourced (C-010, C-012, C-013) | Medium | Medium | Used only to motivate D-010; every behaviour tested is from Tier-1 manufacturer text or spike S1 | Low |
| R-003 | AGPLv3 licence (forced by JUCE 8, D-001) conflicts with owner's intentions (default was GPLv3) | Medium | Low | Recorded in D-001/A-007 and flagged in the final summary; relicensing only requires a JUCE commercial licence | Low |
| R-004 | Magic self-oscillation produces very loud output (up to ~+12.6 dBFS, T-013 bound) | Medium | High | Bounded by tanh; documented; G-003 question a | Low — matches pedal ("twice as loud") |
| R-005 | Malicious session/preset state crashes host | High | Low | D-016 manual validated parser; T-032 fuzz (2000 cases); pluginval state tests | Low |
| R-006 | NaN/Inf from host corrupts feedback state permanently | High | Low | Input sanitised (D-014); T-023 | Low |
| R-007 | Denormals in decaying tails cause CPU spikes | Medium | Medium | Flush threshold + ScopedNoDenormals; T-024 | Low |
| R-008 | Per-sample resampling + two shifters exceed CPU target, esp. at 192 kHz | Medium | Low | Budget checked by T-060/T-061 in CI; DR-05 | Low |
| R-009 | macOS/Windows toolchain behaviour unverifiable in planning sandbox | Medium | Medium | CI matrix (S-010); D-001 fallback rule to JUCE 9.0.3; DR-07, DR-09, DR-11 | Low-Medium |
| R-010 | Unity-at-2-o'clock level law (C-004) is single-source, V1 text | Low | Medium | Calibration `kUnityKnob` adjustable at G-003 | Low |
| R-011 | "A third above" could mean minor third (+3) | Low | Medium | A-013; `kPitchUpSemitones` adjustable at G-003 | Low |
| R-012 | Fresh-context reviews were not independent (single model, A-015) | Medium | Medium | Mechanical linter (plan/tools/lint_plan.py), red verification by compiler/runtime, explicit role-switched passes | Medium |
| R-013 | ASan interposes operator new, conflicting with rt_tests' replacement | Low | Medium | ENVIRONMENT.md fallback: sanitize dsp_tests only, logged | Low |
| R-014 | Planning PAT exposed in chat transcript | High | — | Never written to repo; owner must revoke it after planning (stated in final message and ENVIRONMENT.md §Credentials) | Depends on owner |
| R-015 | Windows ctest `bash` resolves to WSL | Medium | Medium | DR-11 PATH fix | Low |
| R-016 | Frozen-file hashes differ on CRLF checkouts (Windows) | Medium | Medium | `.gitattributes eol=lf` (S-001) + check_freeze.py normalises CRLF | Low |
| R-017 | Public repo already exposes the plan/code (repository is public) | Low | — | No secrets committed; no binaries released (G-002) | Low |
