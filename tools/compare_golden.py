# Compares the transcribed coordinate tables of the original HORCOM7P
# (tests/golden/chartNN_original_koordinaten.json) against the output of
# the new CLI for the same charts (chartNN_new_koordinaten.txt).
# Original longitudes carry arc seconds, houses arc minutes. Regenerate
# the new side with horcom.exe using the settings in tests/golden/README.md.
import json, re, sys, os

GOLD = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tests", "golden")
SIGNS = ["AR", "TA", "GM", "CN", "LE", "VI", "LI", "SC", "SG", "CP", "AQ", "PS"]

def lon_deg(d, sign, m, s=0.0):
    return SIGNS.index(sign) * 30 + d + m / 60.0 + s / 3600.0

def parse_new(cid):
    path = os.path.join(GOLD, f"chart{cid}_new_koordinaten.txt")
    bodies = {}
    houses = {}
    armc = None
    jd = None
    for line in open(path, encoding="utf-8"):
        # the CLI prints the uppercase pl$ names, older recordings the
        # lowercase sprite tags, both parse
        m = re.match(r"^(so|mo|me|ve|ma|ju|sa|ur|ne|pl|dr|ds|ac|mc)\s+(\d+)\s+(\w\w)\s+(\d+)'(\d+)\"\s*([-+0-9.]*)\s*([-+0-9.]*)\s*([-+0-9.]*)", line, re.IGNORECASE)
        if m:
            b = m.group(1).lower()
            bodies[b] = {
                "lon": lon_deg(int(m.group(2)), m.group(3), int(m.group(4)), int(m.group(5))),
                "lat": float(m.group(6)) if m.group(6) else None,
                "dekl": float(m.group(7)) if m.group(7) else None,
                "vel": float(m.group(8)) if m.group(8) else None,
            }
            continue
        m = re.match(r"^\s*(\d+)\s+(\d+)\s+(\w\w)\s+(\d+)'(\d+)\"", line)
        if m:
            houses[int(m.group(1))] = lon_deg(int(m.group(2)), m.group(3), int(m.group(4)), int(m.group(5)))
            continue
        m = re.search(r"ARMC\s+([0-9.]+)", line)
        if m: armc = float(m.group(1))
        m = re.search(r"JD\(UT\)\s+([0-9.]+)", line)
        if m: jd = float(m.group(1))
    return {"bodies": bodies, "houses": houses, "armc": armc, "jd": jd}

# The original built its true node on a lunar latitude that carried the
# nutation in obliquity, a documented bug the rewrite fixes. Its printed
# node therefore sits up to about 95 arc seconds from the rewrite's, which
# lies about 3.5 times closer to the Swiss Ephemeris osculating node, see
# tests/golden/README.md. The node rows get that allowance
NODE_TOL_LON_SEC = 100.0
NODE_TOL_DEKL = 0.02


def compare(cid, tol_lon_sec=1.5, tol_house_min=1.0, tol_dekl=0.01, tol_lat=0.01):
    orig = json.load(open(os.path.join(GOLD, f"chart{cid}_original_koordinaten.json"), encoding="utf-8"))
    new = parse_new(cid)
    report = []
    ok = True
    for b, o in orig.get("bodies", {}).items():
        n = new["bodies"].get(b)
        if not n:
            report.append(f"  {b}: missing in new output"); ok = False; continue
        od = lon_deg(o["lon"][0], o["lon"][1], o["lon"][2], o["lon"][3] if len(o["lon"]) > 3 else 0)
        d = abs(od - n["lon"]) * 3600
        if d > 360 * 3600 - 5400: d = abs(d - 360 * 3600)
        tol = tol_lon_sec if len(o["lon"]) > 3 else 35.0
        node = b in ("dr", "ds")
        if node:
            tol = max(tol, NODE_TOL_LON_SEC)
        line = f"  {b}: lon diff {d:6.1f}\""
        if d > tol: line += "  <-- MISMATCH"; ok = False
        if o.get("lat") is not None and n["lat"] is not None:
            dl = abs(o["lat"] - n["lat"])
            line += f"  lat {dl:.4f}"
            if dl > tol_lat: line += " <-- LAT"; ok = False
        if o.get("dekl") is not None and n["dekl"] is not None:
            dd = abs(o["dekl"] - n["dekl"])
            line += f"  dekl {dd:.4f}"
            if dd > (NODE_TOL_DEKL if node else tol_dekl): line += " <-- DEKL"; ok = False
        report.append(line)
    for h, o in orig.get("houses", {}).items():
        n = new["houses"].get(int(h))
        if n is None: continue
        od = lon_deg(o[0], o[1], o[2])
        d = abs(od - n) * 60
        if d > 21000: d = abs(d - 21600)
        line = f"  H{h}: diff {d:5.2f}'"
        if d > tol_house_min: line += "  <-- MISMATCH"; ok = False
        report.append(line)
    if orig.get("armc_hms") and new["armc"] is not None:
        h, m, s = orig["armc_hms"]
        oa = (h + m / 60 + s / 3600) * 15
        da = abs(oa - new["armc"]) * 3600 / 15
        line = f"  ARMC diff {da:.1f}s"
        if da > 1.5: line += "  <-- MISMATCH"; ok = False
        report.append(line)
    if orig.get("jd") and new["jd"]:
        dj = abs(orig["jd"] - new["jd"]) * 86400
        line = f"  JD diff {dj:.1f}s"
        if dj > 1.0: line += "  <-- MISMATCH"; ok = False
        report.append(line)
    return ok, report

if __name__ == "__main__":
    ids = sys.argv[1:]
    if not ids:
        ids = sorted(set(re.findall(r"chart(\d+)_original_koordinaten\.json",
                                    " ".join(os.listdir(GOLD)))))
    allok = True
    for cid in ids:
        ok, rep = compare(cid)
        allok &= ok
        print(f"chart {cid}: {'OK' if ok else 'DIFFERENCES'}")
        for line in rep:
            print(line)
    sys.exit(0 if allok else 1)
