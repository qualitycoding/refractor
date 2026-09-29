# 3.6 Cold read

Mechanical check (Haiku-equivalent): `python3 plan/tools/lint_plan.py` → `LINT OK` (all S/T/D/DR/A/C/G/R/SC IDs defined;
all 12 template fields present in every step; every evidence test exists, is frozen, and is produced by a step).

Semantic cold read (role-switched pass reading only HANDOFF.md + plan/, A-015). Questions raised and resolved in the plan:
1. "How exactly is the chorus LFO applied?" → D-012 formula added.
2. "What tone filter topology? When is it bypassed?" → D-007 formula added.
3. "Smoother coefficient? Bypass ramp end state?" → D-014 formulas; exact 0/1 end state for bitwise T-015/T-035.
4. "Where does feedback tap from and which tick?" → S-007 action 1 (previous internal tick, post-level, tanh).
5. "How do I get `main` for the PR?" → S-014 action 3.
6. "Where are pluginval hashes for macOS/Windows?" → ENVIRONMENT.md §Pinned artefacts.
7. "Windows bash for ctest scripts?" → DR-11.
Unanswerable-without-asking count after fixes: 0.
