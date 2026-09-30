# Spike S3 — frequency dependence of the two-tap delay-line shifter (found during S-006, 2026-09-30)

Build (from repo root): `g++ -std=c++20 -O2 -Isrc -Itests research/spikes/S3-frequency-dependence/centroid.cpp src/dsp/PitchShifter.cpp -o cen && ./cen`

Result (centroid-output.txt): three independent pitch estimators (FFT peak, spectral centroid, zero-crossing rate) agree
that the bare shifter of D-010 mistunes its output, by an amount that depends on the INPUT FREQUENCY:
| input | ratio 0.75 | ratio 0.375 | ratio 2.52 |
|---|---|---|---|
| 440 Hz | −0.6 % | −3.0 % | +1.1 % |
| 300 Hz | −2.2 % | −11.1 % | +4.0 % |
| 220 Hz | +4.2 % | +21.2 % | −7.7 % |
The original Python spike (S1/spike.py `shifter`) reproduces the 220/300/440 Hz numbers, so the C++ port is faithful.
S1 measured only a 440 Hz input, which is why C-020 looked clean. See ../../ERRATA.md.
