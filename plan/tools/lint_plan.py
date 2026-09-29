"""3.6 mechanical cold-read check: every referenced ID is defined; every PLAN step has all template fields;
every step's evidence tests exist in TRACEABILITY; every test file in TRACEABILITY exists and is frozen."""
import json, re, pathlib, sys
R = pathlib.Path(__file__).resolve().parents[2]; err = []
read = lambda p: (R / p).read_text()
docs = {p: read(p) for p in ["HANDOFF.md","plan/PLAN.md","plan/DECISIONS.md","plan/ASSUMPTIONS.md","plan/GATES.md",
        "plan/TRACEABILITY.md","plan/ENVIRONMENT.md","premortem/RISK_REGISTER.md","research/QUESTIONS.md"]}
defined = {
 "S": set(re.findall(r"^### (S-\d{3})", docs["plan/PLAN.md"], re.M)),
 "D": set(re.findall(r"\*\*(D-\d{3})", docs["plan/DECISIONS.md"])),
 "DR": set(re.findall(r"^\| (DR-\d{2})", docs["plan/DECISIONS.md"], re.M)),
 "A": set(re.findall(r"^\| (A-\d{3})", docs["plan/ASSUMPTIONS.md"], re.M)),
 "G": set(re.findall(r"^## (G-\d{3})", docs["plan/GATES.md"], re.M)),
 "T": set(re.findall(r"^\| (T-\d{3})", docs["plan/TRACEABILITY.md"], re.M)),
 "SC": set(re.findall(r"^\| (SC-\d+)", docs["plan/TRACEABILITY.md"], re.M)),
 "R": set(re.findall(r"^\| (R-\d{3})", docs["premortem/RISK_REGISTER.md"], re.M)),
 "C": {c["id"] for c in json.loads(read("research/claims.json"))},
}
for p, t in docs.items():
    for k, ids in defined.items():
        pat = rf"\b({k}-\d{{2,3}})\b" if k in ("DR","SC") else rf"(?<![A-Z]){k}-\d{{3}}\b"
        for m in set(re.findall(pat, t)):
            m = m if isinstance(m, str) else m[0]
            if m not in ids: err.append(f"{p}: undefined {m}")
fields = ["Tier:","Profile:","Depends on:","Inputs:","Actions:","Outputs:","Evidence produced:","Done when:","Checkpoint:","On failure:","Gate:","Relevant decisions/claims:"]
for blk in re.split(r"^### ", docs["plan/PLAN.md"], flags=re.M)[1:]:
    sid = blk.split()[0]
    for f in fields:
        if f"- {f}" not in blk: err.append(f"PLAN {sid}: missing field {f}")
    for t in re.findall(r"T-\d{3}", blk.split("- Evidence produced:")[1].split("\n")[0]):
        if t not in defined["T"]: err.append(f"PLAN {sid}: evidence {t} not in TRACEABILITY")
manifest = read("tests/FROZEN_MANIFEST.sha256") if (R/"tests/FROZEN_MANIFEST.sha256").exists() else ""
for line in docs["plan/TRACEABILITY.md"].splitlines():
    m = re.match(r"\| (T-\d{3}) \| [^|]+ \| ([^|]+) \|", line)
    if m:
        f = m.group(2).strip().split(" ")[0]
        if "/" in f and not (R / f).exists(): err.append(f"{m.group(1)}: file {f} missing")
        if "/" in f and f not in manifest: err.append(f"{m.group(1)}: file {f} not frozen")
        body = "".join((R/q).read_text() for q in [f] if "/" in f and (R/q).exists())
        if body and m.group(1) not in body and "check_perf" not in f: err.append(f"{m.group(1)}: ID not found in {f}")
for t in sorted(defined["T"]):
    if not any(t in blk for blk in [docs["plan/PLAN.md"]]): err.append(f"{t} not produced by any step")
print("\n".join(err) if err else "LINT OK"); sys.exit(1 if err else 0)
