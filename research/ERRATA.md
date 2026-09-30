# Errata

## E-001 — C-020 overstated (2026-09-30)
C-020 ("a two-tap triangular-crossfade delay-line shifter reproduces requested ratios within 0.61 % for −5..+4 st")
is true only for a 440 Hz sine, the only input frequency spike S1 used. Spike S3 (research/spikes/S3-frequency-dependence)
shows errors of several percent up to +21 % at other input frequencies (220–300 Hz) and larger shifts. C-020's
confidence should be read as "verified at one frequency only". Consequences: BLOCKED.md. The statement "accurate to
about 0.6 %" in the planning summary given to the owner was wrong in general.

**Resolution (2026-09-30):** owner chose a phase-vocoder shifter (option C). Spike S4 (research/spikes/S4-phase-vocoder) verifies
<0.01 % pitch error at every tested frequency and ratio (C-023), chord fidelity vs resolution (C-024), exact lag N/f_int (C-022,
revised) and the output crest factor (C-025). The frozen tests now include a frequency sweep (T-025) and a chord test (T-026)
so this class of error cannot pass unnoticed. See plan/amendments/AMENDMENTS.md.
