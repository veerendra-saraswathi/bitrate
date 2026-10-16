# Bitrate POC – Status (17 Oct 2026)

## What works
- Pure C/C++ encoder + decoder (zero external runtime libraries)
- Tile-based (64×64) reversible 5/3 integer wavelet
- Uniform quantisation (QP knob)
- Zero-run + compact residual entropy coder
- Compact metadata header (magic, dimensions, bit-depth, QP, tile size)
- File-based round-trip
- Perfect reconstruction at qp=1 on real KAIST thermal and visible frames
- Measured encode latency on MacBook

## Results on KAIST preview (640×512, 8-bit grayscale)

| Modality | QP | Encoded size | vs raw (327680 B) | Max error | Encode time |
|----------|----|--------------|-------------------|-----------|-------------|
| Thermal  | 1  | 294854 B    | 1.11×            | 0         | 32.2 ms    |
| Thermal  | 4  | 149296 B    | 2.19×            | 7         | 26.7 ms    |
| Visible  | 1  | 587599 B    | 0.56× (expands)  | 0         | 57.0 ms    |
| Visible  | 4  | 222535 B    | 1.47×            | 10        | 31.2 ms    |

## Next steps (priority order)
1. Stronger entropy coding so visible also compresses at qp=1
2. 10/12/14/16-bit path
3. Colour (3-channel) support
4. Ethernet/UDP framing
5. Jetson Xavier NX / Orin Nano build + latency measurement
6. Windows decoder binary
7. Adaptive rate control driven by channel bandwidth
8. Formal ≤2-frame latency test harness

## Build & run
```bash
mkdir build && cd build
cmake ..
make -j
./encoder ../data/kaist_thermal.raw 640 512 1
./decoder encoded.bin ../data/kaist_thermal.raw