# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
"""T-060/T-061 [performance] Runs the bench 5x per scenario; median realtime fraction must be <= target.
Targets (A-011): 48 kHz/128 <= 0.05; 192 kHz/128 <= 0.20. Median-of-5 absorbs CI noise; spread reported.
Arg 1: path to refractor_bench. Enforces: SC-7."""
import json, statistics, subprocess, sys
bench = sys.argv[1]; ok = True
for sr, target, tid in ((48000, 0.05, "T-060"), (192000, 0.20, "T-061")):
    runs = []
    for _ in range(5):
        r = subprocess.run([bench, str(sr), "128", "60"], capture_output=True, text=True)
        if r.returncode != 0: print(f"{tid} FAIL: bench exited {r.returncode}: {r.stderr.strip()[:300]}"); sys.exit(1)
        runs.append(json.loads(r.stdout)["realtime_fraction"])
    med = statistics.median(runs); good = med <= target; ok &= good
    print(f"{tid} sr={sr} median={med:.4f} min={min(runs):.4f} max={max(runs):.4f} target<={target} {'OK' if good else 'FAIL'}")
sys.exit(0 if ok else 1)
