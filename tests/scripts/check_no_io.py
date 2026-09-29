# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-051 [security] Plugin/DSP code performs no network, process, or arbitrary file I/O (threat model A-010).
Enforces: SC-8, D-019."""
import re, sys, pathlib
root = pathlib.Path(sys.argv[1]); bad = []
pat = re.compile(r"\b(juce::URL|URL::|WebInputStream|StreamingSocket|DatagramSocket|ChildProcess|std::system|popen|"
                 r"std::ifstream|std::ofstream|fopen|FileInputStream|FileOutputStream|curl_|getaddrinfo)\b")
for f in sorted((root / "src").rglob("*")):
    if f.suffix in {".cpp", ".hpp", ".h", ".ipp", ".mm"}:
        for n, line in enumerate(f.read_text(errors="replace").splitlines(), 1):
            if pat.search(line): bad.append(f"{f.relative_to(root)}:{n}: {line.strip()}")
print("\n".join(bad) if bad else "T-051 OK"); sys.exit(1 if bad else 0)
