# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-053 [legal] No third-party trademarks or product names in shipped sources/resources/metadata (D-018, A-003).
Scans src/, resources/ (if present) and CMakeLists.txt. Planning docs are exempt. Enforces: SC-9."""
import re, sys, pathlib
root = pathlib.Path(sys.argv[1]); bad = []
pat = re.compile(r"rainbow\s*machine|earth\s*quaker|\beqd\b|flexi-?switch", re.I)
paths = [root / "CMakeLists.txt"] + [p for d in ("src", "resources") if (root / d).exists() for p in (root / d).rglob("*") if p.is_file()]
for f in paths:
    try: text = f.read_text(errors="strict")
    except Exception: continue
    for n, line in enumerate(text.splitlines(), 1):
        if pat.search(line): bad.append(f"{f.relative_to(root)}:{n}: {line.strip()}")
print("\n".join(bad) if bad else "T-053 OK"); sys.exit(1 if bad else 0)
