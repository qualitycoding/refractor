# Round 1 — Decompose (R1) + Breadth (R2)
- Built question tree (research/QUESTIONS.md) with Q1–Q8.
- Breadth searches: pedal controls (EQD page, 6 retailer listings), internal architecture (FV-1 articles, Dirtbox, madbean,
  FV-1 ROM table), toolchain versions via GitHub API (JUCE, Catch2, pluginval, CMake).
- New claims: C-001..C-014, C-016, C-017, C-031, C-032, C-036.
- Key finding: V1 architecture reportedly = FV-1 ROM pitch program + analogue regeneration + clock-pot Tracking (C-010, C-012, C-013).
  Suggests D-010 (variable-clock delay-line shifter). V2 architecture undocumented (C-017).
- Key finding: V1 and V2 Secondary semantics differ (C-007 contradiction entry) → V2 chosen per A-004.
