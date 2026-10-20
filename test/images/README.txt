# Test Images

These files are for verifying the bitrate encoder/decoder after cloning.

## synthetic_64x64.raw
- Dimensions: 64 × 64
- Format: 8-bit unsigned monochrome raw (one byte per pixel, no header)
- Content: simple ramp (pixel value = index & 0xFF)
- Size: 4096 bytes
- Purpose: matches the in-memory data used by test_roundtrip.cpp

## How to regenerate the synthetic image (if the .raw file is missing)

From the repository root:

```bash
python3 scripts/generate_test_images.py
```

This creates `test/images/synthetic_64x64.raw`.

## How to use with the tools (after building)

```bash
# From the build directory
./encoder 64 64 ../test/images/synthetic_64x64.raw out.bite   # once real file input is added
./decoder out.bite reconstructed.raw
```

Currently the command-line encoder generates its own synthetic frame internally.
The .raw file is provided so you can later feed real (or this synthetic) data once file loading is implemented.

## Adding more images

Place additional raw or simple-format frames here and document them in this file
(width, height, bit depth, channels, source).
