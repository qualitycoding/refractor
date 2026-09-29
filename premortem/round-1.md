# Pre-mortem round 1 (role-switched passes by the planning model; A-015)
Premise: "It is six months later and the implementation failed. What caused it?"

| # | Lens | Finding | Severity | Fix applied |
|---|---|---|---|---|
| 1 | Software eng. | JUCE 7 default cannot build on macOS 15 runners | Critical | D-001 → JUCE 8.0.15 (found in R5; confirmed here) |
| 2 | Test adequacy | T-040 used a non-existent pluginval flag (`--validate-in-process`) → integration test could never pass | Critical | Script corrected before freeze (C-037) |
| 3 | Security/legal | Class name `FlexiSwitch` would ship a registered third-party mark; T-053 would fail forever | High | Renamed `DualModeSwitch`; T-053 checks mark |
| 4 | Operational | Windows ctest may invoke WSL bash → pluginval test fails for environmental reasons | High | DR-11, PLAN S-010 |
| 5 | Operational | CRLF checkout on Windows changes frozen-file hashes → T-070 fails | High | `.gitattributes` in S-001; check_freeze normalises CRLF |
| 6 | Handoff | Chorus LFO, tone filter, smoothing and bypass formulas were prose-only → implementer guesses → bitwise tests (T-015, T-021, T-035) fail | High | Exact formulas added to D-007, D-012, D-014 |
| 7 | Test adequacy | T-008 monotonicity assertion initially inverted | High | Fixed before freeze |
| 8 | Handoff | Bench declared vectors via most-vexing parse → harness wouldn't compile | High | Fixed before freeze (red verification caught it) |
| 9 | Security | State restore via `replaceState` on unvalidated trees | Medium | D-016 + S-009 action 2 |
| 10 | Test adequacy | Perf test on shared CI runners is noisy | Medium | Median of 5, spread reported (check_perf.py) |
| 11 | Operational | Implementer lacks PR/CI credentials | Medium | A-016, ENVIRONMENT.md §Credentials |
| 12 | Software eng. | Empty repo: gen branch became default; `main` absent | Medium | S-014 creates `main` before PR |
| 13 | Fidelity | No objective fidelity criterion | Medium | Accepted by intake (A-005); G-003 loop; R-001 |
Critical/High findings: 8 → all fixed. Round 2 required.
