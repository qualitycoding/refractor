# Round 3 — Adversarial (R5) + Empirical (R6)
Adversarial pass (role-switched, same model — A-015) attempted to falsify each load-bearing claim:
- **C-(default) JUCE 7 viable → FALSIFIED.** Found C-030 (juceaide vs macOS 15 SDK). D-001 switches to JUCE 8.0.15; licence
  consequence AGPLv3 recorded. This is the only design-changing finding of the round.
- pluginval CLI: `--validate-in-process` (used in the first T-040 draft) does not exist in 1.0.4 → script fixed before freeze (C-037).
- Trademark check: "Flexi-Switch" is a registered EQD mark on the product page → class renamed `DualModeSwitch`, T-053 scans for it.
- C-004 (unity at 2 o'clock): both retail sources share identical copy → counted as one source; stays single-source (R-010).
- C-012/C-013: sources are hobbyist and madbean-derived; cannot reach corroborated → carried as risk R-002, design does not depend on them being exact (calibration + G-003).
- Empirical (S2): JUCE 8.0.15 + CMake 3.31.10 configure/build of stub plugin + tests on Ubuntu 24.04; red verification (see spikes/S2-env).
Saturation check for next round: no new load-bearing claims pending, no unresolved contradictions except documented V1/V2 difference.
→ Research terminated after round 3 (minimum met, saturated). Remaining below-bar claims → RISK_REGISTER (R-001, R-002, R-010, R-011).
