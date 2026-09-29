# Profile record (Rule 0)

| Item | Value |
|---|---|
| Active profiles | `software` |
| `software.deploys` | `false` — no service is operated and no public release is in scope (A-011). CI artifacts only. |
| `math.exploration` | n/a (`math` inactive) |

Justification: the deliverable is a C++ audio plugin (code intended to run and be maintained). No mathematical
statements are published (`math` off); DSP correctness is tested as software behaviour rather than published as numerical
results (`computational` off, see D-021); no manuscript or archive deposit (`publication` off, A-005).

## Not-applicable sections (produce no artifacts, impose no checks)
- 0.3.3 Mathematical intake; 0.3.4 Computational intake; 0.3.5 Publication intake
- R2b Novelty & prior-art search; `research/NOVELTY.md`
- 2A (all: 2A.0–2A.5) — `math/` tree absent
- 2B.2 Provenance contract; 2B numerical V&V categories; 2B.4 unknown-outcome tests
- 2C Figure specification — `figures/` absent
- 2D Manuscript specification — `manuscript/` absent
- `plan/OPERATIONS.md` (only if `software.deploys`)
- Rule 7 evidence classes; Rule 8 result-agnostic planning (fidelity uncertainty is instead handled by calibration constants + gate G-003)
- Gate G-001 (math/computational only)
- Phase 4 lenses: mathematical, numerical, novelty, publication; deployment-test and operational-fork categories that require `software.deploys`
