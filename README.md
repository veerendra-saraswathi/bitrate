# Bitrate — RCI SoW Problem 4

Pure-C++ intra-frame encoder/decoder exploration for low-bitrate, low-latency thermal/visible frames on Jetson.

## Internal evaluation (landing page)

**Current assessment register and scores:**  
[docs/INTERNAL_EVALUATION.md](docs/INTERNAL_EVALUATION.md)

- Architecture E (bitrate2 / final ewc codec): **82 / 100** (provisional, host)
- Architecture F: **Deferred** (exactness closed, 2 Mbps budget not met)

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

## Status

See [docs/INTERNAL_EVALUATION.md](docs/INTERNAL_EVALUATION.md) for the latest architecture ranking, evidence, and next actions.
