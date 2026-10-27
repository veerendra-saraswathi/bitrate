# Jetson Xavier NX / Orin Nano – Smoke Build Checklist

## 1. Native build on the Jetson (simplest)
```bash
# On the Jetson
sudo apt update
sudo apt install -y build-essential cmake git
git clone https://github.com/veerendra-saraswathi/bitrate.git
cd bitrate
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
## MacBook proxy baseline (2026-10-27)
levels=1, mode=1, 640x512 KAIST thermal, 200 repeats

| QP | enc median | enc p95 | dec median | FPS (enc) |
|----|------------|---------|------------|-----------|
| 2  | 28.8 ms   | 32.6 ms | 35.7 ms   | ~35      |
| 8  | 29.9 ms   | 43.1 ms | 38.0 ms   | ~33      |
| 12 | 28.0 ms   | 36.3 ms | 35.8 ms   | ~36      |
| 24 | 29.5 ms   | 47.9 ms | 39.3 ms   | ~34      |

These are laptop numbers. Re-run the same `bench_jetson` binary on Jetson Orin Nano for real figures.
