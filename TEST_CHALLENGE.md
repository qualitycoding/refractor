# TEST_CHALLENGE (DR-02) — frozen tests that are wrong. Halted; no frozen file was edited.

All items are my own (planning-phase) errors. Evidence was produced against the real engine; scratch copies with the fixes
below pass (T-015, T-016, T-018 and every other audio/operational case, 77 assertions, under ASan+UBSan).

## TC-1 T-015 — toggle condition is unreachable (tests/dsp/test_engine_audio.cpp:147-148)
`half = size_t(0.5*48000) = 24000`; blocks start at multiples of 128 (…23936, 24064), so `if (p == half)` never fires.
Bypass is never engaged; the test then asserts out == in and fails (max diff 0.3568) — correctly, because nothing was bypassed.
**Fix:** `if (p >= half && p - half < 128) e.setParameter(ParamId::Bypass, 1.0f);` (timing checks below it stay valid).

## TC-2 T-016 — same unreachable toggle (tests/dsp/test_engine_audio.cpp:165-166) → vacuous pass
No parameter step is ever applied, so the six "smoothing" checks compare steady-state output with itself. T-016 currently
passes without verifying anything. **Fix:** same condition as TC-1 with `e.setParameter(id, 1.0f)`. With the fix it passes.

## TC-3 T-018 — last block overruns its buffers (tests/dsp/test_engine_operational.cpp:19,22,25,29)
Signals are 0.5 s = 24000 samples (= 187.5 blocks) but `process(..., 128)` is always called with 128, so the final call reads
and writes 64 samples past the end of a, b, o0, o1 and x. Observed: glibc `malloc(): corrupted top size`, SIGABRT in the
test binary. **Fix:** pass `int(std::min<size_t>(128, a.size() - p))` (x.size() on line 29) as the block length.
T-035 (plugin test) and T-022 were audited: their loop conditions already prevent overrun.

## TC-4 T-009 — tolerance not supported by evidence (tests/dsp/test_engine_audio.cpp `kRatioTolOct`)
The 2.5 % tolerance was derived from spike S1's 1.82 % at −12 st. T-009 knob 0…0.4 gives secondary ratios 0.375…0.472
(−17…−13 st), where even the bare shifter is off by 2.0–3.0 % at 440 Hz (measured 3.35 % in the engine, knob 0). Do NOT just
widen the tolerance: see BLOCKED.md, where the same root cause shows far larger errors at other input frequencies.

## Needed from owner
Permission to amend the frozen files as above (and regenerate tests/FROZEN_MANIFEST.sha256 + re-run plan/tools/lint_plan.py),
ideally together with the decision in BLOCKED.md so the tests are amended once.
