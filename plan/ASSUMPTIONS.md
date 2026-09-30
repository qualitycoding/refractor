# Assumptions (A-###)

Intake batch sent 2026-09-28. The human answered item 1 (push access: supplied a fine-grained PAT) and item 2
(repository `qualitycoding/refractor`, confirming the name "Refractor"). All other items adopt the proposed default.

| ID | Item | Resolution | Source |
|---|---|---|---|
| A-001 | Push access | Human-supplied fine-grained PAT used by the planning agent only, via a credential helper outside the repo; never written to any file. Implementer uses its own credential (see ENVIRONMENT.md §Credentials). | human |
| A-002 | Repository | `https://github.com/qualitycoding/refractor` (was empty). Pushing the generation branch to an empty repo makes it the default branch; the implementer creates `main` in S-001. | human |
| A-003 | Name & IP | Product name "Refractor". No EarthQuaker Devices trademarks (incl. "Rainbow Machine", "Flexi-Switch"), artwork, or trade dress in code, UI, binaries or metadata. Enforced by T-053 and G-003 checklist. README may state it is "inspired by a classic FV-1-era pitch-warping pedal" without naming it. | default |
| A-004 | Target version | Behaviour of the current (V2) pedal as described by the manufacturer (C-001..C-011, C-015, C-016). V1-only behaviour (bipolar Secondary knob) is out of scope. | default |
| A-005 | Fidelity target | Behavioural reproduction from published control descriptions; no reference captures available. Subjective closeness judged by the human at G-003 by ear. No null-test/spectral-match criterion. | default |
| A-006 | Profiles | `software` only, `software.deploys=false`. | default |
| A-007 | Toolchain | Default was C++20 + JUCE 7 + CMake (GPLv3). **Superseded by D-001** (JUCE 8.0.15, AGPLv3) because JUCE 7 cannot build on macOS 15 CI runners (C-030). | default → D-001 |
| A-008 | Formats/platforms | VST3 + Standalone on Windows x64, macOS arm64, Linux x64. No AU, AAX, CLAP, iOS. | default |
| A-009 | Signal path | Mono or stereo in; mono or stereo out; 44.1–192 kHz; clean exact dry path; no analog emulation. | default |
| A-010 | Controls / threat model | Every knob and switch is a host-automatable parameter; expression = separate "Pitch Exp" parameter + "Exp Enabled" switch (MIDI-CC mapping is done by the host). Threat model: untrusted inputs are host-supplied audio buffers, parameter values, and state blobs (session files may come from other people). No network, files, or processes are touched by the plugin. Data sensitivity: none (no personal data). | default |
| A-011 | Performance & release | ≤5 % of one core at 48 kHz/128 (worst-case settings, median of 5 on `ubuntu-24.04` runner); ≤20 % at 192 kHz. Latency reported 0 (the lag is the effect). No public release; any release waits behind G-002. Versioning SemVer starting 0.1.0; maintenance: best-effort by owner. | default |
| A-012 | Research budget | 3–6 rounds; pitch-shift method chosen during research (D-010). | default |
| A-013 | "A third above" | Interpreted as a major third (+4 semitones); coincides with the FV-1 ROM pitch program limit (C-014). Adjustable only via `kPitchUpSemitones` at G-003 (3 or 4). | planning agent |
| A-014 | Level law | Unity gain at knob 0.7 ("about 2 o'clock" on a 300° pot). Source is V1 retail copy (C-004); kept for V2. | planning agent |
| A-015 | Model tiers / subagents | Planning ran in a single chat session with one model available and no subagent spawning. All "fresh-context" roles (R5, 3.6, Phase 4) were executed as separate, explicitly role-switched passes by the same model; logged as tier substitutions in `.checkpoints/state.json`. This weakens independence (R-012). | environment |
| A-016 | Implementer environment | Implementer has: git, Python ≥3.10, network to github.com/pypi, and a GitHub token with `contents:write` + `actions:read` on the repo (env `GH_TOKEN`). macOS/Windows builds are evidenced only via GitHub Actions. | planning agent |
| A-017 | AI-use disclosure | README states the plan and code were produced with AI assistance (owner preference unknown; disclosure is the reversible choice). | planning agent |
| A-018 | Owner decisions 2026-09-30 | Owner chose option C (phase-vocoder shifter) and approved amending the frozen tests (TEST_CHALLENGE TC-1…TC-4). Recorded in plan/amendments/AMENDMENTS.md. The PAT was re-used from the conversation for the same repository. | human |
