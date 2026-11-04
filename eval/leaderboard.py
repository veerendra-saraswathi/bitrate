#!/usr/bin/env python3
import json
from pathlib import Path

rows = []
for p in Path("eval/results").glob("*.json"):
    rows.append(json.loads(p.read_text()))
rows.sort(key=lambda r: r["score"], reverse=True)
print(f"{'Config':<16} {'Score':>6} {'Mbps':>8} {'PSNR':>6} {'max_err':>8}")
for r in rows:
    print(f"{r['config']:<16} {r['score']:>6.1f} {r['mbps']:>8.2f} {r['psnr']:>6.1f} {r['max_err']:>8}")
