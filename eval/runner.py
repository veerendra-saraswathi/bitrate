#!/usr/bin/env python3
"""
Run one or more labelled configs against a set of raw frames.
Usage:
  python3 eval/runner.py eval/configs/A1.yaml data/kaist_thermal.raw 640 512
  python3 eval/runner.py eval/configs/*.yaml data/*.raw 640 512
"""
import sys, subprocess, time, json, csv
from pathlib import Path
from score import score_run

def run_one(cfg_path, raw_path, w, h):
    cfg = json.loads(Path(cfg_path).read_text()) if cfg_path.endswith(".json") else None
    # simple YAML-less: expect JSON for now
    if cfg is None:
        raise SystemExit("Use JSON configs for now")

    name = cfg["name"]
    profile = cfg.get("profile", "NL")

    if "precomputed" in cfg:
        metrics = cfg["precomputed"]
        s = score_run(metrics, profile)
        return {
            "config": name,
            "frame": Path(raw_path).name,
            "score": s,
            **metrics,
        }

    encoder = cfg["encoder"]
    args = cfg.get("args", [])

    out_bin = Path(f"/tmp/{name}.bin")
    t0 = time.perf_counter()
    subprocess.check_call([encoder, raw_path, str(w), str(h)] + args,
                          stdout=subprocess.DEVNULL)
    encode_ms = (time.perf_counter() - t0) * 1000
    size = out_bin.stat().st_size if out_bin.exists() else 0
    # For demo we assume the encoder writes encoded.bin in cwd
    size = Path("encoded.bin").stat().st_size if Path("encoded.bin").exists() else size

    bpp = size * 8 / (w * h)
    mbps = bpp * w * h * 30 / 1e6

    # Real decode + metrics
    subprocess.check_call(["./build/decoder", "encoded.bin"],
                          stdout=subprocess.DEVNULL)
    import numpy as np
    orig = np.fromfile(raw_path, dtype=np.uint8).reshape(h, w)
    rec = np.fromfile("reconstructed.raw", dtype=np.uint8).reshape(h, w)
    max_err = int(np.max(np.abs(orig.astype(np.int16) - rec.astype(np.int16))))
    mse = np.mean((orig.astype(np.float64) - rec.astype(np.float64)) ** 2)
    psnr = 10 * np.log10(255**2 / mse) if mse > 0 else 999.0
    bit_exact = max_err == 0

    metrics = {
        "mbps": round(mbps, 2),
        "psnr": round(psnr, 2),
        "max_err": max_err,
        "encode_ms": round(encode_ms, 1),
        "bit_exact": bit_exact,
    }
    s = score_run(metrics, profile)
    return {
        "config": name,
        "frame": Path(raw_path).name,
        "score": s,
        **metrics,
    }

def main():
    if len(sys.argv) < 5:
        print("Usage: runner.py <config.json> <raw> <w> <h>")
        sys.exit(1)
    cfg, raw, w, h = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
    result = run_one(cfg, raw, w, h)
    print(json.dumps(result, indent=2))

    out = Path("eval/results") / f"{result['config']}.json"
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(result, indent=2))
    print(f"Wrote {out}")

if __name__ == "__main__":
    main()
