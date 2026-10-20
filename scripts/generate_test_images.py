#!/usr/bin/env python3
"""Generate a small synthetic 8-bit mono raw test image for the bitrate repo."""

from pathlib import Path

OUT_DIR = Path(__file__).resolve().parent.parent / "test" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)

width, height = 64, 64
pixels = bytes((i & 0xFF) for i in range(width * height))

out_path = OUT_DIR / "synthetic_64x64.raw"
out_path.write_bytes(pixels)
print(f"Wrote {out_path} ({len(pixels)} bytes, {width}x{height} 8-bit mono)")
