# HANDOFF — Refractor implementation

**Purpose.** Implement *Refractor*, a VST3 + Standalone audio plugin (JUCE 8.0.15, C++20) that reproduces the published
behaviour of a V2 polyphonic pitch-warping guitar pedal (FV-1-era design), so that the frozen test suite passes on
Linux, macOS and Windows CI and the owner signs off by ear at gate G-003. Name, UI and metadata must not reference the
original product (A-003).

**Active profile:** `software` (`software.deploys=false`). See plan/PROFILE.md for N/A sections.

## Reading order
1. This file → 2. plan/PLAN.md (steps S-001..S-014) → 3. plan/DECISIONS.md (design D-###, rules DR-##) →
4. plan/ASSUMPTIONS.md → 5. plan/ENVIRONMENT.md → 6. plan/GATES.md → 7. plan/TRACEABILITY.md →
8. premortem/RISK_REGISTER.md → 9. src/dsp/*.hpp (public contracts) → 10. research/claims.json as needed.

## Environment setup
Follow plan/ENVIRONMENT.md §Linux setup (verified). macOS/Windows are exercised only in CI.

## Run the frozen suite
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_BASE_DIR=$PWD/.fc
cmake --build build -j"$(nproc)"
xvfb-run -a ctest --test-dir build -E '^perf$' --output-on-failure   # functional, security, integration
ctest --test-dir build -L perf --output-on-failure                     # T-060/T-061
```
Single Catch2 case: `build/tests/refractor_dsp_tests "[T-011]"`.

## Verify the freeze
`python3 tests/scripts/check_freeze.py .` must print `T-070 OK` (equivalent: `sha256sum -c tests/FROZEN_MANIFEST.sha256`
on an LF checkout). Never edit, skip, xfail or weaken any file listed in the manifest.

## Steps at a glance
S-001 bootstrap · S-002 env + red baseline · S-003 params/mappings · S-004 dual-mode switch · S-005 shifter + clock
domain · S-006 engine core · S-007 Magic + robustness · S-008 performance · S-009 plugin processor/state · S-010 CI ·
S-011 editor · S-012 demo/snapshot tools · S-013 **G-003 listening gate** · S-014 completion (PR, no release).

## Human gates
- **G-003** (end of S-013): listening + IP sign-off. Halt, write `GATE-G-003.md`, wait for `proceed` / `calibrate:` / `proceed-with-rescope:` / `stop`.
- **G-002** (guard): any tag, release, binary upload or marketplace submission. Not part of this plan; never do it without sign-off.

## Halt / deviation protocol
- Unanticipated situation → plan/DECISIONS.md default rule. Log every deviation in `DEVIATIONS.md` (date, step, what, why).
- Anything touching frozen tests, security, data integrity, a public interface, or research integrity → write `BLOCKED.md` (step, evidence, options) and halt.
- A frozen test you believe is wrong → write `TEST_CHALLENGE.md` (test ID, evidence, proposed fix) and halt; the planning protocol is re-run.

## Integrity rule (Rule 9, verbatim)
> **Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures. In addition:
> * `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
> * `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

(Only the `[All]` sentence applies to this `software`-only plan.)

## Security notes
Never commit tokens. `GH_TOKEN` is read from the environment only. The plugin must not perform network, process or file I/O (T-051).
