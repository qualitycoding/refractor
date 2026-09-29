# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-050 [security/supply-chain] Every FetchContent dependency is pinned to a full 40-hex commit SHA that
matches plan/ENVIRONMENT.md's lock table, and CI actions are pinned by commit SHA. Enforces: SC-8, C-031."""
import re, sys, pathlib
root = pathlib.Path(sys.argv[1]); errs = []
dep = (root / "cmake/Dependencies.cmake").read_text()
tags = re.findall(r"GIT_TAG\s+(\S+)", dep)
if not tags: errs.append("no GIT_TAG entries")
for t in tags:
    if not re.fullmatch(r"[0-9a-f]{40}", t): errs.append(f"GIT_TAG not a full SHA: {t}")
env = (root / "plan/ENVIRONMENT.md").read_text()
for t in tags:
    if t not in env: errs.append(f"GIT_TAG {t} missing from plan/ENVIRONMENT.md lock table")
wf = root / ".github/workflows"
if not wf.exists(): errs.append(".github/workflows missing (created by S-010)")
else:
    for f in wf.glob("*.yml"):
        for m in re.finditer(r"uses:\s*([^\s#]+)", f.read_text()):
            ref = m.group(1)
            if ref.startswith("./"): continue
            if not re.search(r"@[0-9a-f]{40}$", ref): errs.append(f"{f.name}: action not SHA-pinned: {ref}")
print("\n".join(errs) if errs else "T-050 OK"); sys.exit(1 if errs else 0)
