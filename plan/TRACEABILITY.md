# Traceability

## Success criteria
| SC | Criterion (measurable) | Evidence | Step(s) |
|---|---|---|---|
| SC-1 | VST3 + Standalone build on ubuntu-24.04, macos-15 (arm64), windows-2022; VST3 passes pluginval strictness 10 on each | CI `build-test` jobs; T-040 | S-010, S-011 |
| SC-2 | Controls behave as published (pitch range at ANY input frequency, chord fidelity, lag vs Tracking, secondary octave logic, trail direction, expression) | T-002, T-006, T-008, T-009, T-011, T-017, T-025, T-026 | S-003, S-006, S-007 |
| SC-3 | Parameter IDs/ranges/defaults and state are stable and round-trip | T-001, T-030, T-031 | S-003, S-009 |
| SC-4 | Both footswitches latch/momentary; bypass exact and click-free; expression input | T-036, T-015, T-017, T-030 | S-004, S-006, S-011 |
| SC-5 | Dry path bit-transparent when wet is off | T-007 | S-006 |
| SC-6 | 44.1–192 kHz; layouts 1→1, 1→2, 2→2 | T-019, T-018, T-034 | S-006, S-009 |
| SC-7 | ≤5 % core @48k/128, ≤20 % @192k; no RT allocation; latency 0, tail 10 s | T-060, T-061, T-022, T-033 | S-007, S-008, S-009, S-010 |
| SC-8 | Robust to hostile audio/state; pinned supply chain; no I/O; sanitizer-clean | T-023, T-032, T-050, T-051, T-052, T-040 | S-007, S-009, S-010 |
| SC-9 | No third-party marks/trade dress in shipped artefacts | T-053; G-003 question c | S-001, S-011, S-013 |
| SC-10 | Owner listening sign-off | G-003 `proceed` | S-013 |

## Tests → requirement
| Test | Category | File | Requirement(s) | Claims/Decisions |
|---|---|---|---|---|
| T-001 | unit | tests/dsp/test_params_mapping.cpp | SC-3 | D-003 |
| T-002 | unit | tests/dsp/test_params_mapping.cpp | SC-2 | C-003, D-004 |
| T-003 | unit | tests/dsp/test_params_mapping.cpp | SC-2 | C-004, D-005 |
| T-004 | unit | tests/dsp/test_params_mapping.cpp | SC-2 | C-009, D-010 |
| T-005 | unit | tests/dsp/test_params_mapping.cpp | SC-2 | C-005, C-006, D-007, D-008 |
| T-006 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-003, C-023 |
| T-007 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-5 | D-002 |
| T-008 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-009, C-022 |
| T-009 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-007, D-006 |
| T-010 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-011, D-012 |
| T-011 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-015, C-021 |
| T-012 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-006, D-008 |
| T-013 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-8 | C-006, C-025, D-008 |
| T-014 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-005, D-007 |
| T-015 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-4 | C-008, D-013 |
| T-016 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-4 | D-014 |
| T-017 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2, SC-4 | C-002, D-009 |
| T-018 | integration | tests/dsp/test_engine_operational.cpp | SC-6 | D-014, D-017 |
| T-019 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-6 | D-010 |
| T-020 | operational | tests/dsp/test_engine_operational.cpp | SC-3 | D-014 |
| T-021 | operational | tests/dsp/test_engine_operational.cpp | SC-7 | D-014 |
| T-022 | operational/perf | tests/dsp/test_realtime_alloc.cpp | SC-7 | D-014 |
| T-023 | security/operational | tests/dsp/test_engine_operational.cpp | SC-8 | D-014, D-019 |
| T-024 | operational | tests/dsp/test_engine_operational.cpp | SC-7 | D-008 |
| T-025 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-003, C-023, D-010 |
| T-026 | unit (audio) | tests/dsp/test_engine_audio.cpp | SC-2 | C-024, D-010 |
| T-030 | integration | tests/plugin/test_plugin.cpp | SC-3, SC-4 | D-003, D-015 |
| T-031 | integration | tests/plugin/test_plugin.cpp | SC-3 | D-016 |
| T-032 | security | tests/plugin/test_plugin.cpp | SC-8 | D-016, D-019 |
| T-033 | integration | tests/plugin/test_plugin.cpp | SC-7 | D-015 |
| T-034 | integration | tests/plugin/test_plugin.cpp | SC-6 | D-017 |
| T-035 | integration | tests/plugin/test_plugin.cpp | SC-3 | D-015 |
| T-036 | unit | tests/dsp/test_dualmode_switch.cpp | SC-4 | C-008, D-013 |
| T-040 | integration/operational | tests/scripts/run_pluginval.sh | SC-1, SC-8 | C-037 |
| T-050 | security (supply chain) | tests/scripts/check_pins.py | SC-8 | C-031, C-036 |
| T-051 | security | tests/scripts/check_no_io.py | SC-8 | D-019 |
| T-052 | security | CI job `sanitizers` (dsp_tests + rt_tests with REFRACTOR_SANITIZE=ON) | SC-8 | D-019, D-020 |
| T-053 | legal | tests/scripts/check_ip.py | SC-9 | A-003, D-018 |
| T-060 | performance | tests/scripts/check_perf.py + bench/ | SC-7 | A-011 |
| T-061 | performance | tests/scripts/check_perf.py + bench/ | SC-7 | A-011 |
| T-070 | freeze | tests/scripts/check_freeze.py | all | 2E |
| T-071 | completeness | tests/scripts/check_no_stubs.py | all | D-### interfaces |

Deployment-test category: N/A (`software.deploys=false`).
