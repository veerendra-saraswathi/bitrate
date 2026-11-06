# Architecture F — Block-coded reversible 5/3 + adaptive Rice

Pure C++17 starter for the RCI SoW Problem 4 comparison.

## Build
```bash
cd experiments/F
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

## Run (from build/)
```bash
./bench_kaist ../../data/kaist_thermal.raw ../../data/kaist_visible.raw
```

Reports the locked metrics: Mbps @ 30 fps, PSNR, max_err, bit-exact, encode time.

## Status
MVP skeleton: reversible 5/3 (1 level), 32×32 independent blocks, adaptive Rice.
Next: multi-level, better Rice/rANS, rate control for 2 Mbps point, Jetson timing.
