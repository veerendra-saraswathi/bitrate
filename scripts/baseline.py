#!/usr/bin/env python3
import subprocess, statistics, os

def run_encode(raw, w, h, qp=1, bit_depth=8, channels=1, repeats=11):
    times = []
    cmd = ["./encoder", raw, str(w), str(h), str(qp), str(bit_depth), str(channels)]
    for i in range(repeats):
        try:
            out = subprocess.check_output(cmd, cwd="build", text=True, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print("Encode failed:", e.output)
            return
        for line in out.splitlines():
            if "Encode time:" in line:
                ms = float(line.split()[2])
                times.append(ms)
                break
    if len(times) < 2:
        print("Not enough timing samples")
        return
    times.sort()
    times = times[1:]          # drop warmup
    def pct(p):
        if not times:
            return 0.0
        k = (len(times) - 1) * p / 100.0
        f = int(k)
        c = min(f + 1, len(times) - 1)
        return times[f] + (times[c] - times[f]) * (k - f)
    print(f"{w}x{h} ch={channels}  n={len(times)}")
    print(f"  median {statistics.median(times):.1f} ms")
    print(f"  p95    {pct(95):.1f} ms")
    print(f"  p99    {pct(99):.1f} ms")
    print(f"  min/max {min(times):.1f} / {max(times):.1f} ms")

if __name__ == "__main__":
    print("=== Baseline (multi-threaded encoder) ===")
    run_encode("../data/kaist_thermal.raw", 640, 512)
    if os.path.exists("data/frame1920.raw"):
        run_encode("../data/frame1920.raw", 1920, 1200)
    else:
        print("data/frame1920.raw not found – skipping large frame")