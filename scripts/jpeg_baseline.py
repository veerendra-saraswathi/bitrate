#!/usr/bin/env python3
"""
JPEG baseline comparison for the bitrate MVP.
Requires: Pillow (pip install Pillow)
Produces CSV: results/jpeg_baseline_<name>.csv
"""

import sys
import os
from pathlib import Path

try:
    from PIL import Image
    import io
except ImportError:
    print("Pillow not installed. Run: pip install Pillow")
    sys.exit(1)

def load_raw_mono(path, width, height):
    data = Path(path).read_bytes()
    expected = width * height
    if len(data) < expected:
        raise ValueError(f"Short read: {len(data)} < {expected}")
    img = Image.frombytes("L", (width, height), data[:expected])
    return img

def jpeg_curve(img, qualities=range(10, 96, 5)):
    rows = []
    pixels = img.size[0] * img.size[1]
    for q in qualities:
        buf = io.BytesIO()
        img.save(buf, format="JPEG", quality=q, optimize=True)
        nbytes = buf.tell()
        bpp = (nbytes * 8.0) / pixels
        rows.append((q, nbytes, bpp))
    return rows

def main():
    if len(sys.argv) < 4:
        print("Usage: jpeg_baseline.py <raw> <width> <height> [name]")
        print("Example: jpeg_baseline.py data/kaist_visible.raw 640 512 visible")
        sys.exit(1)

    raw_path = sys.argv[1]
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    name = sys.argv[4] if len(sys.argv) > 4 else "frame"

    img = load_raw_mono(raw_path, width, height)
    rows = jpeg_curve(img)

    out_dir = Path("results")
    out_dir.mkdir(exist_ok=True)
    out_path = out_dir / f"jpeg_baseline_{name}.csv"

    with open(out_path, "w") as f:
        f.write("quality,bytes,bpp\n")
        for q, nbytes, bpp in rows:
            f.write(f"{q},{nbytes},{bpp:.5f}\n")

    print(f"Wrote {out_path}")
    print("quality,bytes,bpp")
    for q, nbytes, bpp in rows[:6]:
        print(f"{q},{nbytes},{bpp:.5f}")
    print("...")

if __name__ == "__main__":
    main()
    