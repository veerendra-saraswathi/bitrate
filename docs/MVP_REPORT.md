# Bitrate MVP Report (v0.2)

**Tag:** `mvp-v0.2`  
**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Date:** 2026-10-28

## Summary

Intra-frame low-bitrate encoder/decoder for Jetson Orin Nano (encoder) and Linux/Windows PC (decoder).  
Pure C++17, no external runtime libraries.  
Perfect reconstruction at QP=1 is a hard requirement and is met.

## Delivered

| Item | Status |
|------|--------|
| Cascading 5/3 wavelet + exact round-trip | Done |
| Entropy coding (mode 1: varint + zero-run) | Done |
| Bitstream v0.2 (64-byte versioned header) | Done |
| CSV-driven rate control (KAIST VIS + IR curves) | Done |
| Public C++ API (`include/bitrate.h`) | Done |
| Integrity test (qp=1 bit-exact) | Done |
| Multi-frame regression suite | Done |
| MacBook proxy latency (~30 ms encode on 640×512) | Done |
| Real Jetson Orin Nano numbers | Pending hardware |

## Quality–bitrate (levels=1, mode=1)

- KAIST visible 640×512: QP 32 ≈ 3.92 bpp  
- KAIST thermal 640×512: QP 32 ≈ 2.06 bpp  
- QP 1 is bit-exact on both.

## Tests

```bash
./build/test_roundtrip
./build/test_cascade
./build/test_integrity
./build/test_api
./build/test_multiframe
./build/rate_control_demo visible 4.0
