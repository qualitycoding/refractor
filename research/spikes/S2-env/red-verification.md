# 2B.5 Red verification (planning sandbox, 2026-09-28)

Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.31.10, Ninja 1.13.0, JUCE 8.0.15, Catch2 v3.16.0, pluginval 1.0.4.
Full log: `ctest-red.log`; build log: `build.log` (bld=0, stub VST3 + Standalone + test executables built).

| Harness | Tests | Result against stubs | Clean? |
|---|---|---|---|
| dsp_tests | T-001..T-021, T-023, T-024, T-036 (24 cases) | 24/24 failed; 42/42 assertions "due to unexpected exception" with `refractor::NotImplemented` messages | yes |
| rt_tests | T-022 | failed, NotImplemented (Engine ctor) | yes |
| plugin_tests | T-030..T-035 | 6/6 failed, NotImplemented | yes |
| pluginval | T-040 | failed: plugin loads, `RefractorProcessor` ctor throws NotImplemented → abort (rc 134) | yes (failure attributable solely to stub) |
| check_pins | T-050 | failed: ".github/workflows missing (created by S-010)" | yes |
| check_no_stubs | T-071 | failed: stubs remain | yes |
| perf | T-060/T-061 | failed: bench exits 2 "bench error: Engine::Engine" | yes |
| check_freeze | T-070 | failed before manifest existed; passes after 2E freeze (guard) | n/a — guard |
| check_no_io | T-051 | **passes** against stubs | guard (see below) |
| check_ip | T-053 | **passes** against stubs | guard (see below) |

**Documented exception (D-022):** T-051, T-053 and T-070 are *regression guards* on invariants the stubs already satisfy
(no I/O calls, no third-party marks, frozen files unchanged). A guard that fails on stubs would be testing the stubs, not
the requirement. They were validated instead by mutation: inserting `std::ifstream f("x");` into src/dsp/Engine.cpp made
T-051 fail; inserting a comment containing the pedal's name into src/dsp/Params.cpp made T-053 fail; editing one byte of
a frozen file made T-070 fail (see mutation-check.txt). No test failed because of a syntax error, missing symbol, or
harness problem.
