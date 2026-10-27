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