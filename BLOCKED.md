# BLOCKED — design decision needed (touches research integrity + frozen tests). Halted at S-006/S-007.

## What happened
Implementing S-005/S-006, T-009 failed on the octave-down voice. Investigation (research/spikes/S3-frequency-dependence):
the two-tap triangular-crossfade delay-line shifter chosen in D-010 **mistunes its output by an amount that depends on the
input frequency**. Three independent estimators (FFT peak, spectral centroid, zero-crossing rate) agree, and the original
Python spike reproduces it, so it is neither a measurement artefact nor a porting bug.

| input | ratio 0.75 (−5 st) | ratio 0.375 | ratio 1.26 (+4 st) | ratio 2.52 |
|---|---|---|---|---|
| 440 Hz | −0.6 % | −3.0 % | +0.4 % | +1.1 % |
| 300 Hz | −2.2 % | −11.1 % | +1.4 % | +4.0 % |
| 220 Hz | +4.2 % | +21.2 % | −2.6 % | −7.7 % |

Size of the error ≈ a fraction of the window rate f_w = (1−r)·f_int/W (8 Hz at r = 0.75, W = 1024); on a low guitar note
that is well over a semitone. Window shape (triangular vs raised-cosine) makes no measurable difference; W = 512 is much worse.

## Why I did not catch it
Spike S1 used a single input frequency (440 Hz), which happens to be a good case. C-020 ("within 0.61 %") was therefore
overstated (research/ERRATA.md E-001), and my summary to the owner ("accurate to about 0.6 %") was wrong in general.
All frozen pitch tests also use 440 Hz (T-011 uses 300 Hz but only checks direction), so they cannot detect this — they
would pass while the plugin is audibly out of tune on other notes.

## State of the work (branch impl/refractor-v0.1)
Done and passing against the frozen suite: S-003 (params, mappings: T-001–T-005), S-004 (switch: T-036), S-005 (shifter,
clock domain), S-006/S-007 engine incl. Magic loop: T-006, 007, 008, 010–014, 017, 019–021, 023, 024, rt_tests (T-022).
Failing for reasons that are NOT engine bugs: T-015, T-018 (+T-016 vacuous) → TEST_CHALLENGE.md TC-1…3; T-009 → TC-4.
Not started: S-008 perf (engine is cheap; expect far below target), S-009 plugin, S-010 CI, S-011 editor, S-012 tools, S-013 gate.
None of those depend on which shifter is used, but CI cannot go fully green until the tests are amended.

## Options (my recommendation: run a frequency-sweep spike on B and C first, then choose)
A. Keep D-010 as is; treat the mistuning as the lo-fi character of cheap delay-line shifters; decide by ear at G-003.
   Cheapest. Risk: harmony voices audibly out of tune on low notes; contradicts "polyphonic pitch shifter" expectations.
B. Phase-aligned taps (WSOLA/SOLA-style: choose each tap jump to maximise waveform similarity). Fixes single notes well;
   weaker on chords. Moderate work; keeps lag ≈ W/2 so most tests stand.
C. STFT phase-vocoder shifter at the internal clock rate. Accurate and polyphonic; lag ≈ window, so Tracking still maps to
   lag via the clock. Largest change: re-opens D-010 (which rejected it as "too clean"), T-008/T-012 expectations, CPU budget.
D. C for accuracy with deliberate low-fi degradation (low clock, no anti-aliasing) to keep the pedal's character.
Whatever is chosen, add acceptance tests that sweep input frequency (e.g. 110, 220, 330, 440, 880 Hz and a three-note chord)
so this class of error cannot pass unnoticed again.

## What I need from the owner
1. Choose A/B/C/D (or ask me to run the sweep spike first). 2. Approve amending the frozen tests (TEST_CHALLENGE.md).
