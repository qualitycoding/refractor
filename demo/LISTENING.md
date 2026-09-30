# What to listen for (G-003)

Use headphones. Files are **not normalised**: set your volume on `01_noon_chorus.wav` and leave it. `08` is pre-trimmed by −12 dB
(the raw plugin output at max Magic is +11.4 dBFS). Each file is dry + wet exactly as the plugin outputs it.

## The one fact that frames everything
Lag and chord clarity are tied: **lag × frequency resolution = 1**. A 31 ms lag resolves partials ≈32 Hz apart; 81 ms ≈12 Hz; 208 ms ≈5 Hz.
So no setting gives both "tight" and "clean on chords/bass". Your job is to pick *where on that curve* Tracking should start and end.
`kWindowSamples` only scales the lag range (1024: 31–208 ms; 2048: 62–417 ms; 512: 16–104 ms). It does not change clarity at a given lag.
Bandwidth/grit depends on Tracking alone (internal clock 32.8 kHz at CW → 4.9 kHz at CCW).

## 1. Lag range — `S1_tracking_sweep_chord.wav`, `S2_tracking_sweep_bass.wav`, `02_slapback.wav`
Five segments, same material, Tracking 1 → 0: starts at 0 / 4.5 / 9 / 13.5 / 18 s = lag **31 / 50 / 81 / 130 / 208 ms**.
Decide: (a) is 31 ms tight enough at the CW end, or do you want shorter (`kWindowSamples=512` → 16 ms)? (b) Is 208 ms too long at CCW,
or not long enough? Shorten the long end with `kClockScaleMin` (0.25 → 125 ms); lengthen everything with `kWindowSamples=2048`.
Compare with `N2048_*` files (same knob settings, double lag).

## 2. Chord clarity — S1, `03_fourth_down_harmony.wav`, `N2048_03_…`
Expect at 31 ms: the shifted chord sounds smeared/warbly (G triad ≈ 42 % clean); by ≈ 81 ms it is clean. Is the smear a pleasing "character" at
the tight end, or a flaw? `extras/WETONLY_03…` removes the dry so you can hear the shifter alone.

## 3. Low notes — S2, `09_bass_fourth_down.wav`
Bass (55–82 Hz) needs the longest lag to stay distinct; at short lag it turns indistinct/grumbly. Acceptable, or should the useful range favour low notes?

## 4. "Digital remnants" (grit, aliasing, dullness) — segments at low Tracking; `05`, `06`, `07`
As Tracking falls the wet gets darker and grittier (slower internal clock, deliberate aliasing). Is that the right amount? (`kClockScaleMin` 0.10–0.50:
lower = slower, dirtier, longer.) Also listen for phase-vocoder artefacts: watery/"phasey" sustain and smeared pick attacks. Judge: ok / too clean / too smeared.

## 5. Always-on detune — `01_noon_chorus.wav`
At noon the wet is ±8 cents at 0.6 Hz, which should sound like a chorus. Too subtle, or seasick? (`kChorusDepthCents` 2–20.)

## 6. Pitch range and balance — `03`, `04_third_up_plus_octave.wav`
Pitch CCW is −5 st ("a fourth below"), CW is +4 st (major third); Secondary adds the octave above (04). Right intervals? Is wet vs dry balance right at Primary 0.7 (unity)?
(`kPitchDownSemitones`, `kPitchUpSemitones`, `kUnityKnob`.)

## 7. Magic — `05_magic_low_repeats`, `06_magic_ascending`, `07_magic_descending`, `08_max_self_oscillation_trimmed-12dB`
05: a few short, fading repeats (≈ 0.4 s of regeneration): enough ambience? 06/07: each pass shifts further, up or down, and fades — does the climb/fall feel right, and is it too short
(06/07 wet lasts ≈ 1.8 s)? 08: self-oscillation that wanders inharmonically and sustains. Does max Magic feel too tame or too wild (`kMagicMaxFeedback` 0.9–1.5)?
**Loudness:** untrimmed, 08 reaches +11.4 dBFS. Keep the ceiling (`kWetCeiling` 4.0 = +12 dBFS) or lower it (2.0 = +6 dBFS)? Below half the ceiling nothing is altered.

## Not isolated in these files
Tone (`kToneMinCutoffHz`) and Expression. Ask if you want a sweep.

## How to answer
Per file: ok / too clean / too dirty / wrong lag / wrong pitch. Then one of: `proceed` · `calibrate: kWindowSamples=2048, kClockScaleMin=0.25, …` ·
`proceed-with-rescope: …` · `stop`. Fidelity caveat: the pedal's DSP is unpublished and there are no reference captures, so this is judged by your ear.
