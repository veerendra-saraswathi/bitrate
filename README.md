# bitrate

Early POC skeleton for a low-bitrate, low-latency intra-frame image encoder/decoder intended for Jetson Xavier NX / Orin Nano (encoder) and Linux/Windows PC (decoder).

The current code implements:

- A reversible integer 5/3 lifting wavelet transform
- A self-describing bitstream format (magic `0xB17E0001` / "BITE", frame header with width, height, tile size, QP, etc.)
- Placeholder encode/decode functions
- A unit test that verifies the wavelet round-trip on synthetic data

Image loading, tile looping, quantisation integration, Golomb-Rice entropy coding, and real command-line tools are still TODO.

## After Cloning

```bash
git clone https://github.com/veerendra-saraswathi/bitrate.git
cd bitrate
```

### Dependencies

- CMake ≥ 3.16
- C++17 compiler (GCC, Clang, or MSVC)
- No external libraries required for the current skeleton

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This produces:

- `libbitrate_core` (or equivalent static/shared library)
- `test_roundtrip` executable (the only fully working binary at present)

**Note:** `CMakeLists.txt` also declares `encoder` and `decoder` targets that reference `src/encoder_main.cpp` and `src/decoder_main.cpp`. Those files do not yet exist, so those targets will fail to build until the mains are added. The core library and the round-trip test build cleanly.

### Run the Test

From the `build` directory:

```bash
./test_roundtrip
```

Expected output:

```
5/3 wavelet round-trip OK
```

The test uses a synthetic 64×64 tile (values 0–255) generated in memory. No external image files are required.

## Where to Find / Place Images for Testing

There are **no sample images** checked into the repository.

Recommended layout for colleagues who want to add real images:

```
bitrate/
├── test/
│   ├── test_roundtrip.cpp      # current synthetic test
│   └── images/                 # ← create this folder and drop test images here
│       ├── frame_1920x1200.raw
│       ├── frame_1920x1200.pgm
│       └── ...
├── data/                       # alternative location for larger datasets
└── ...
```

**Current status of image support:**

- The encoder/decoder functions expect raw pixel buffers (`std::vector<uint8_t>` or `std::vector<int32_t>`).
- No PGM/PNG/JPEG loader is implemented yet.
- For quick experiments you can:
  1. Generate raw 8-bit mono frames of the desired size, or
  2. Use the existing synthetic path in `test/test_roundtrip.cpp` as a template and extend it to load a file.

Once image I/O is added, the convention will be to look in `test/images/` first.

## Current Bitstream Format (self-contained)

The header written by `BitstreamWriter` contains at least:

- Magic: `0xB17E0001`
- Width, height
- Bit depth, channels
- Tile size (default 128)
- QP
- Sequence number and timestamp

This makes the bitstream self-describing for a decoder on another machine.

## Next Implementation Steps (for the team)

1. Add `src/encoder_main.cpp` and `src/decoder_main.cpp` with simple CLI:
   - Encoder: `encoder <input.raw> <width> <height> <output.bite> [qp]`
   - Decoder: `decoder <input.bite> <output.raw>`
2. Implement the tile loop, quantisation, and Golomb-Rice (or similar) entropy coding inside `encode_frame` / `decode_frame`.
3. Add a minimal raw/PGM loader so real images from `test/images/` can be used.
4. Measure sustained frame rate and end-to-end latency on Jetson.

## Design Context

This skeleton follows the design analysis that prioritises:

- Intra-frame (no inter-frame for lowest latency and error resilience)
- Independently decodable tiles
- Integer 5/3 lifting wavelet
- Self-contained bitstream with minimal dependencies
- Suitability for Jetson edge constraints

See the accompanying engineering analysis document for the full rationale on bitrate, rate control, entropy coding, and latency.

---

**Status:** Initial POC skeleton (commit message: "Initial POC skeleton for low-bitrate encoder/decoder").  
Ready for colleagues to build, run the wavelet test, and begin extending with real image pipelines.
