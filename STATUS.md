# Bitrate MVP v0.1.0 – Frozen Baseline (18 Oct 2026)

## Implemented
- Pure C/C++17 encoder + decoder (zero external runtime libraries)
- Tile-based (64×64) reversible 5/3 integer wavelet
- Uniform quantisation (QP)
- Zero-run + compact residual entropy
- 8-bit and 16-bit mono
- 3-channel colour (independent planes)
- Compact metadata header
- UDP framing + loopback reassembly
- Multi-threaded tile encode
- Perfect reconstruction at qp=1 (mono 8/16-bit + colour)
- Real KAIST thermal + visible tested

## Measured results (MacBook)
| Frame                   | QP | Channels | Size out | Time    | UDP packets |
|-------------------------|----|----------|----------|---------|-------------|
| KAIST 640×512 thermal   | 1  | 1        | 295 KB   | ~20 ms  | 211         |
| Synthetic 1920×1200     | 1  | 1        | 6.41 MB  | ~290 ms | 4581        |
| Synthetic RGB 640×512   | 1  | 3        | ~2.74 MB | ~208 ms | 1955        |

16-bit synthetic: 0 mismatches.

## Build & test
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
./encoder ../data/kaist_thermal.raw 640 512 1 8 1
./test_roundtrip