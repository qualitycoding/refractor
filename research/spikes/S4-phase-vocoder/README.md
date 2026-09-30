# Spike S4 — phase-vocoder shifter (owner decision "1c", 2026-09-30)

Build any program from the repo root, e.g.
`g++ -std=c++20 -O2 -Isrc -Itests research/spikes/S4-phase-vocoder/pv_sweep.cpp src/dsp/PitchShifter.cpp -o pv_sweep && ./pv_sweep 1024`
(`lagsweep.cpp`, `crest.cpp`, `bench_t1.cpp` also link the engine sources: see the g++ lines used in plan/amendments/AMENDMENTS.md).

| file | finding |
|---|---|
| sweep-output.txt | Pitch error 0.00 % at every input frequency (82–1200 Hz) and ratio (0.375–2.52); rms gain 0.91–1.00 (exactly 1.00 at ratio 1; one outlier 0.79 at 82 Hz → 31 Hz). Supports C-023. |
| chord-lag-cpu-output.txt | Lag by energy centroid = N samples exactly at ratio 1 (N−3 at 1.26); ~290 ns/sample/voice. Supports C-022 (revised). |
| chord-resolution-output.txt | Chord partials survive intact when bin width f_int/N ≲ 16 Hz (≲ 8 Hz for close low chords); lag × bin width = 1. Supports C-024. |
| lag-tone-output.txt | Engine lag = N/f_int to <0.2 ms for 220–440 Hz tone bursts at every Tracking; random-noise bursts jitter ±2 ms (measurement, not engine). |
| crest-output.txt | With max Magic the phase-vocoder loop peaks at up to 1.7× the delay-line bound → explicit wet ceiling (kWetCeiling, D-008). After the ceiling: ≤1.03 × the old bound and ≤ |dry| + ceiling by construction. |
| cpu-worst-case-*.txt | Tracking = 1, both voices, Magic max: 2.3 % (48 kHz/128) and 2.5 % (192 kHz/128) of one core (sandbox, 1 vCPU). |
