# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-071 [completeness] No NotImplemented stub remains in src/ except the type definition in src/dsp/Errors.hpp."""
import sys, pathlib
root = pathlib.Path(sys.argv[1]); bad = []
for f in sorted((root / "src").rglob("*")):
    if f.is_file() and f.name != "Errors.hpp" and "NotImplemented" in f.read_text(errors="replace"):
        bad.append(str(f.relative_to(root)))
print("stubs remain in: " + ", ".join(bad) if bad else "T-071 OK"); sys.exit(1 if bad else 0)
