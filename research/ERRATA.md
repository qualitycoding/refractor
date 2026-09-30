# Errata

## E-001 — C-020 overstated (2026-09-30)
C-020 ("a two-tap triangular-crossfade delay-line shifter reproduces requested ratios within 0.61 % for −5..+4 st")
is true only for a 440 Hz sine, the only input frequency spike S1 used. Spike S3 (research/spikes/S3-frequency-dependence)
shows errors of several percent up to +21 % at other input frequencies (220–300 Hz) and larger shifts. C-020's
confidence should be read as "verified at one frequency only". Consequences: BLOCKED.md. The statement "accurate to
about 0.6 %" in the planning summary given to the owner was wrong in general.
