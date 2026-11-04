# JPEG-LS Pure C/C++ Port — Design Note

**Status:** Draft for implementation  
**Date:** 2026-11-02  
**Goal:** Self-contained, standards-compliant JPEG-LS encoder (and later decoder) with zero external runtime dependencies.

## 1. Scope of the first implementation

| Item | Decision |
|------|----------|
| Component count | Single-component only |
| Bit depth | 8-bit and 14-bit grayscale (SoW range 8/10/12/14/16) |
| Mode | Lossless only (NEAR = 0) |
| Interleave | None |
| Colour transform | None |
| Marker stream | Full JPEG-LS baseline markers (SOI, SOF55, SOS, EOI, optional DNL) |
| Entropy coding | Standard JPEG-LS Golomb/Rice + run mode |
| Reset | Standard RESET procedure |
| Private extensions | None in v1 |
| External libraries | Forbidden |

The bitstream must be decodable by a compliant JPEG-LS decoder (CharLS, libjpeg, etc.).

## 2. Non-goals for the first increment

- Near-lossless (NEAR > 0)
- Multi-component / colour
- Line-interleaved or sample-interleaved modes
- Custom preset coding parameters (use JPEG-LS defaults)
- Decoder (encoder first; decoder follows once bitstream is validated)
- Rate control or tiling
- Jetson-specific optimisations

## 3. Public API (proposed)

```c
// include/jpegls.h
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jpegls_encoder jpegls_encoder;

jpegls_encoder* jpegls_encoder_create(void);
void jpegls_encoder_destroy(jpegls_encoder* enc);

// bit_depth: 8..16, components must be 1 for v1
int jpegls_encoder_set_frame_info(jpegls_encoder* enc,
                                  uint32_t width,
                                  uint32_t height,
                                  uint8_t bit_depth,
                                  uint8_t components);

// NEAR must be 0 for the first implementation
int jpegls_encoder_set_near(jpegls_encoder* enc, int near_lossless);

// Encode raw planar grayscale into a JPEG-LS byte stream.
// Returns number of bytes written, or negative on error.
int jpegls_encoder_encode(jpegls_encoder* enc,
                          const void* samples,
                          size_t sample_count,
                          uint8_t* destination,
                          size_t destination_capacity);

#ifdef __cplusplus
}
#endif
