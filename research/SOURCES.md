# Sources (accessed 2026-09-28)

| Tier | Source | Used for | Verification note |
|---|---|---|---|
| 1 | EarthQuaker Devices, *Rainbow Machine* product page (V2 text), earthquakerdevices.com/rainbow-machine | C-001..C-011, C-015, C-016 | Fetched in full; controls section read verbatim. Manual PDF linked there (EQD-EU-Rainbow-Machine-R3.pdf) not fetched — same publisher, not independent. |
| 1 | Spin Semiconductor FV-1 datasheet, Table 1 "ROM Programs" (mirror experimentalnoize.com/manuals/FV-1.pdf) | C-014 | Table text in search extract; consistent with PedalPCB quote. |
| 1 | JUCE repository at tags 7.0.12, 8.0.15, 9.0.3: LICENSE.md, README.md, CMakeLists.txt, docs/Linux Dependencies.md | C-031, C-032, C-034, C-035 | Read raw files at pinned tags. |
| 1 | `git ls-remote` of JUCE and Catch2 | C-031, C-036 | Commit SHAs recorded. |
| 1 | actions/runner-images README + images tree | C-033 | Labels and deprecation badge read directly. |
| 1 | pluginval v1.0.4 release asset + `--help` | C-037 | Binary downloaded, sha256 recorded, help captured in research/spikes/S2-env. |
| 1 | Spike S1 (research/spikes/S1-delayline-shifter) | C-015, C-020..C-022 | Deterministic, re-runnable (`python3 spike.py`). |
| 1 | Spike S2 (research/spikes/S2-env) | C-038 | Build/red-verification logs. |
| 2 | JUCE GitHub issue #1427 | C-030 | Issue body read (search extract). |
| 3 | Retail listings: Coast Sonic, Brian's Guitars (identical V1 copy — counted as ONE source), Guitar Brothers (V2), Chicago Music Exchange (V2), Rusty's Cool Guitars | C-001..C-009, C-011 | Used only to corroborate the Tier-1 manufacturer text. |
| 3 | Reverb.com, "The Revolutionary Chip Inside…" (2023-12-13) | C-012 | Names Rainbow Machine as FV-1 based. |
| 4 | Dirtbox Layouts post + C. Stelloh comments (2024) | C-010, C-012, C-013 | Hobbyist trace derived from madbean's schematic; not independent of madbean. Never sufficient alone (R-002). |
| 4 | madbeanpedals forum ("Rainbow Puker") | C-012, C-013 | Author traced the PCB; "exploiting one of the internal patches". |
| 4 | JUCE forum FYI thread on juceaide + Xcode 16 | C-030 | Corroborates issue #1427. |
| 4 | PedalPCB forum quoting FV-1 Table 1 | C-014 | Matches datasheet. |

Method references (background, not load-bearing, no locators claimed): J. Dattorro, "Effect Design Part 2: Delay-Line
Modulation and Chorus", J. Audio Eng. Soc. 45(10), 1997; U. Zölzer (ed.), *DAFX: Digital Audio Effects*, 2nd ed., Wiley 2011.
The load-bearing algorithm claims rest on spike S1, not on these texts.
