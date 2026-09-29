# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-070 Freeze integrity: every line of tests/FROZEN_MANIFEST.sha256 matches (portable sha256sum -c)."""
import hashlib, sys, pathlib
root = pathlib.Path(sys.argv[1]); bad = []
for line in (root / "tests/FROZEN_MANIFEST.sha256").read_text().splitlines():
    if not line.strip(): continue
    h, p = line.split(None, 1); p = p.lstrip("*").strip(); f = root / p
    if not f.exists(): bad.append(f"missing {p}"); continue
    data = f.read_bytes().replace(b"\r\n", b"\n")
    if hashlib.sha256(data).hexdigest() != h: bad.append(f"changed {p}")
print("\n".join(bad) if bad else "T-070 OK"); sys.exit(1 if bad else 0)
