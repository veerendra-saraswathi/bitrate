#!/usr/bin/env python3
"""
Architecture D reference benchmark: JPEG-LS via imagecodecs (CharLS).
Host-only. Do not integrate into Jetson pure-C++ path.
"""

import time
from pathlib import Path

import numpy as np
import imagecodecs

# Adjust paths/names to match the exact KAIST raw frames you used for A/B/C
TEST_IMAGES = [
    ("kaist_thermal_640x512", "test/images/kaist_thermal_640x512.raw", 640, 512),
    ("kaist_visible_640x512", "test/images/kaist_visible_640x512.raw", 640, 512),
]

NEAR_VALUES = [0, 1, 2, 4]   # 0 = lossless (bit-exact)


def load_raw(path: Path, w: int, h: int) -> np.ndarray:
    data = np.fromfile(path, dtype=np.uint8)
    if data.size != w * h:
        raise ValueError(f"{path} size {data.size} != {w}x{h}")
    return data.reshape(h, w)


def psnr(a: np.ndarray, b: np.ndarray) -> float:
    mse = np.mean((a.astype(np.float64) - b.astype(np.float64)) ** 2)
    if mse == 0:
        return float("inf")
    return 10 * np.log10(255.0 ** 2 / mse)


def bench_one(name: str, path: Path, w: int, h: int, near: int, fps: int = 30):
    img = load_raw(path, w, h)

    # warm-up
    _ = imagecodecs.jpegls_encode(img, level=near)

    t0 = time.perf_counter()
    encoded = imagecodecs.jpegls_encode(img, level=near)
    elapsed = time.perf_counter() - t0

    decoded = imagecodecs.jpegls_decode(encoded)

    bpp = (len(encoded) * 8) / (w * h)
    mbps = bpp * w * h * fps / 1e6
    max_err = int(np.max(np.abs(img.astype(np.int16) - decoded.astype(np.int16))))
    quality = psnr(img, decoded)

    return {
        "image": name,
        "near": near,
        "bytes": len(encoded),
        "bpp": round(bpp, 4),
        "mbps_30fps": round(mbps, 2),
        "psnr": round(quality, 2) if quality != float("inf") else "inf",
        "max_err": max_err,
        "encode_ms": round(elapsed * 1000, 2),
        "bit_exact": max_err == 0,
    }


def main():
    print("image,near,bytes,bpp,mbps_30fps,psnr,max_err,encode_ms,bit_exact")
    results = []
    for name, rel, w, h in TEST_IMAGES:
        path = Path(rel)
        if not path.exists():
            print(f"# SKIP missing {path}")
            continue
        for near in NEAR_VALUES:
            r = bench_one(name, path, w, h, near)
            results.append(r)
            print(
                f"{r['image']},{r['near']},{r['bytes']},{r['bpp']},"
                f"{r['mbps_30fps']},{r['psnr']},{r['max_err']},{r['encode_ms']},{r['bit_exact']}"
            )

    out = Path("results/architecture_D_jpegls.csv")
    out.parent.mkdir(exist_ok=True)
    with out.open("w") as f:
        f.write("image,near,bytes,bpp,mbps_30fps,psnr,max_err,encode_ms,bit_exact\n")
        for r in results:
            f.write(
                f"{r['image']},{r['near']},{r['bytes']},{r['bpp']},"
                f"{r['mbps_30fps']},{r['psnr']},{r['max_err']},{r['encode_ms']},{r['bit_exact']}\n"
            )
    print(f"\nWrote {out}")


if __name__ == "__main__":
    main()
