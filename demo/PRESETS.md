# Demo presets (rendered by tools/render_demos)

48 kHz, 24-bit mono WAV. Output = dry + wet exactly as the plugin produces it; nothing is normalised.
Knob values are 0-1 plugin parameter values; semitones, gain and lag are what they map to.

| file | input | Pitch | Primary | Secondary | Tracking (lag) | Tone | Magic | Magic on | peak | note |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_noon_chorus.wav | pluck_phrase | 0.50 (-0.0 st) | 0.70 (0.0 dB) | 0.00 | 0.80 (46 ms) | 1.00 | 0.00 | no | -3.6 dBFS | unison: the always-on detune makes noon a chorus |
| 02_slapback.wav | pluck_phrase | 0.50 (-0.0 st) | 0.70 (0.0 dB) | 0.00 | 0.20 (143 ms) | 1.00 | 0.00 | no | -5.6 dBFS | Tracking 0.2 = long lag, clearly separate wet copy |
| 03_fourth_down_harmony.wav | chord_G | 0.00 (-5.0 st) | 0.70 (0.0 dB) | 0.00 | 0.50 (81 ms) | 1.00 | 0.00 | no | -6.5 dBFS | Pitch fully CCW (-5 st) on a chord |
| 04_third_up_plus_octave.wav | pluck_phrase | 1.00 (+4.0 st) | 0.70 (0.0 dB) | 0.50 | 0.50 (81 ms) | 1.00 | 0.00 | no | -5.2 dBFS | Pitch CW (+4 st) with Secondary adding the octave above |
| 05_magic_low_repeats.wav | pluck_phrase | 0.50 (-0.0 st) | 0.70 (0.0 dB) | 0.00 | 0.50 (81 ms) | 1.00 | 0.35 | yes | -5.7 dBFS | Magic low at noon: a few regenerating repeats |
| 06_magic_ascending.wav | single_note_A3 | 1.00 (+4.0 st) | 0.70 (0.0 dB) | 0.00 | 0.50 (81 ms) | 1.00 | 0.60 | yes | -6.3 dBFS | Magic with Pitch above noon: trails climb |
| 07_magic_descending.wav | single_note_A3 | 0.00 (-5.0 st) | 0.70 (0.0 dB) | 0.00 | 0.50 (81 ms) | 1.00 | 0.60 | yes | -8.0 dBFS | Magic with Pitch below noon: trails fall |
| 08_max_self_oscillation.wav | single_note_A3 | 0.80 (+2.4 st) | 1.00 (3.1 dB) | 1.00 | 0.30 (118 ms) | 0.60 | 1.00 | yes | 11.4 dBFS | Everything up: bounded self-oscillation (loud by design) **[peak above 0 dBFS: this WAV is hard-clipped; the plugin output is not]** |
| 09_bass_fourth_down.wav | bass_line | 0.00 (-5.0 st) | 0.70 (0.0 dB) | 0.00 | 0.30 (118 ms) | 0.70 | 0.00 | no | -2.1 dBFS | Low notes are where a short window smears most: listen for clarity |
