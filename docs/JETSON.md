# Jetson Xavier NX / Orin Nano notes

## Build
```bash
# On the Jetson (or cross-compile)
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
