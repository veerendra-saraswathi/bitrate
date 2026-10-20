# bitrate

Early POC skeleton for a low-bitrate, low-latency intra-frame image encoder/decoder
intended for Jetson Xavier NX / Orin Nano (encoder) and Linux/Windows PC (decoder).

## After Cloning

```bash
git clone https://github.com/veerendra-saraswathi/bitrate.git
cd bitrate
```

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Test images (where to find them)

Test images live in **`test/images/`**.

After cloning you will find:

- `test/images/README.txt` — describes each file (size, format, purpose)
- `test/images/synthetic_64x64.raw` — 64×64 8-bit mono ramp (or generate it)

If the `.raw` file is missing, generate it:

```bash
python3 scripts/generate_test_images.py
```

### Run the tests

```bash
cd build

# Wavelet correctness (synthetic data, no files needed)
./test_roundtrip
# Expected: 5/3 wavelet round-trip OK

# Encoder / decoder on synthetic data
./encoder 64 64 test.bite
./decoder test.bite test.raw
```

## Current status

- Wavelet 5/3 round-trip is exact.
- Encoder and decoder binaries build and run (header is written/parsed correctly).
- Full compression (tile loop + entropy coding) and real file input are still TODO.
- No external libraries required.

See `test/images/README.txt` for more detail on the sample data.
