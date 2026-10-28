# Bitrate MVP Report (v0.2)

**Tag:** `mvp-v0.2`  
**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Date:** 2026-10-28  
**Jetson runs:** EDGE AI Platform cloud (Orin Nano Super), 2026-10-28

## Summary

Intra-frame low-bitrate encoder/decoder for Jetson Orin Nano (encoder) and Linux/Windows PC (decoder).  
Pure C++17, no external runtime libraries.  
Perfect reconstruction at QP=1 is a hard requirement and is met (numerically and visually).

## Delivered (MVP)

| Item | Status |
|------|--------|
| Cascading 5/3 wavelet + exact round-trip | Done |
| Entropy coding (mode 1: varint + zero-run) | Done |
| Bitstream v0.2 (64-byte versioned header) | Done |
| CSV-driven rate control (KAIST VIS + IR curves) | Done |
| Public C++ API (`include/bitrate.h`) + docs | Done |
| Integrity + multi-frame regression suite | Done |
| JPEG baseline comparison | Done |
| Defense-grade I/O characterisation (multiple contents & sizes) | Done |
| Visual side-by-side inspection | Done |
| Real Jetson Orin Nano latency / FPS | **Done** |
| MacBook proxy latency | Done |

## Jetson Orin Nano Super results (cloud)

**Platform:** NVIDIA Jetson Orin Nano Engineering Reference Developer Kit Super  
R36.4.7, kernel 5.15.148-tegra, aarch64

| Frame | QP | Encode median | Decode median | Approx FPS |
|-------|----|---------------|---------------|------------|
| 640×512 synthetic | 8 | **5.81 ms** | 3.98 ms | **~172** |
| 640×512 synthetic | 1 | **5.67 ms** | 4.06 ms | **~176** |
| 64×64 synthetic | 8 | 0.058 ms | 0.043 ms | ~17 300 |

≤ 2-frame latency SoW bound is comfortably met (5.8 ms ≪ 33 ms at 30 fps).

## Quality–bitrate (levels=1, mode=1)

| Content | Size | QP=1 bpp (exact) | QP=8 bpp | QP=32 bpp |
|---------|------|------------------|----------|-----------|
| KAIST thermal | 640×512 | 6.05 | 3.40 | 2.06 |
| KAIST thermal | 1920×1200 | 4.55 | 3.38 | 2.04 |
| KAIST visible | 640×512 | 13.83 | 4.16 | 3.93 |
| KAIST visible | 1920×1200 | 9.87 | 4.02 | 3.91 |
| edges (synthetic) | 640×512 | 3.64 | 2.51 | 2.50 |
| gradient | 640×512 | 7.05 | 3.88 | 3.50 |
| noise | 640×512 | 20.61 | 15.21 | 12.66 |
| target blob | 640×512 | 4.35 | 4.04 | 4.04 |

JPEG baseline (Pillow) on the same KAIST frames is substantially smaller (e.g. visible Q=30 ≈ 0.41 bpp), as expected for a simple wavelet design.

## Visual inspection

- QP=1: bit-exact on all tested content (thermal street scene, gradient, edges, etc.).
- QP=32 on thermal: heavy blocking / posterisation; scene largely unusable.
- Structured content (edges, targets) survives mid-QP better than natural thermal texture.
- Practical seeker operating point is likely QP 4–12.

## Tests (all passing)

```bash
./build/test_roundtrip
./build/test_cascade
./build/test_integrity
./build/test_api
./build/test_multiframe
./build/rate_control_demo visible 4.0