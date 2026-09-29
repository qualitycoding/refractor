# Freeze record (2E) — not itself frozen

Frozen: every file listed in `tests/FROZEN_MANIFEST.sha256` (tests/**, bench/**). Verify with
`python3 tests/scripts/check_freeze.py .` (CRLF-tolerant) or `sha256sum -c tests/FROZEN_MANIFEST.sha256`.

## Red verification (2026-09-28, Ubuntu 24.04, GCC 13.3, CMake 3.31.10, JUCE 8.0.15, Catch2 v3.16.0)
Evidence: `research/spikes/S2-env/`.
| Suite | Result against stubs | Failure mode |
|---|---|---|
| dsp_tests (T-001..T-021, T-023, T-024, T-036) | 24/24 cases fail | all 42 failing assertions "due to unexpected exception" `refractor::NotImplemented` |
| rt_tests (T-022) | 1/1 fails | NotImplemented |
| plugin_tests (T-030..T-035) | 6/6 fail | NotImplemented (processor constructor) |
| pluginval (T-040) | fails | plugin aborts on NotImplemented during "Open plugin (cold)" |
| check_pins (T-050) | fails | `.github/workflows missing (created by S-010)` |
| check_no_stubs (T-071) | fails | stubs present |
| perf (T-060/T-061) | fails | bench exits 2, NotImplemented |
| check_no_io (T-051), check_ip (T-053) | **pass** | Documented exception D-022: invariant guards; validated by mutation (research/spikes/S2-env/mutation-check.txt). |
| check_freeze (T-070) | passes after manifest creation | By definition. |
| T-052 (sanitizers) | configuration of dsp/rt tests | Fails with them (NotImplemented) under ASan+UBSan. |
No failure is caused by a syntax, import, or missing-file error.
