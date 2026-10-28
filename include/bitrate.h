#pragma once
// bitrate — public API (v0.2)
//
// Stable surface for the low-bitrate intra-frame encoder/decoder.
// All other headers (bitstream.h, wavelet.h, rate_control.h, …) are internal.
//
// Requirements: C++17, no external runtime libraries.

#include <cstdint>
#include <vector>
#include <string>

// ------------------------------------------------------------------
// Frame header (Bitstream v0.2, 64 bytes)
// ------------------------------------------------------------------
struct FrameHeader {
    uint32_t magic       = 0xB17E0001;
    uint16_t version     = 0x0002;
    uint16_t header_size = 64;
    uint16_t width       = 0;
    uint16_t height      = 0;
    uint8_t  bit_depth   = 8;
    uint8_t  channels    = 1;
    uint8_t  tile_size   = 64;
    uint8_t  qp          = 1;
    uint8_t  levels      = 1;
    uint8_t  coder_mode  = 1;   // 0 = zero-run, 1 = varint (recommended)
    uint8_t  flags       = 0;   // bit0 = perfect-reconstruction (qp == 1)
    uint8_t  reserved0   = 0;
    uint32_t sequence    = 0;
    uint64_t timestamp   = 0;
    uint8_t  reserved[32] = {0};
};

// ------------------------------------------------------------------
// Core encode / decode
// ------------------------------------------------------------------

// Encode one frame.
// pixels: row-major 8-bit mono (width * height bytes)
// qp:     1 = perfect reconstruction, higher = coarser
// levels: wavelet levels (1 recommended)
// coder_mode: 1 = varint (recommended), 0 = legacy zero-run
// Returns the complete bitstream (64-byte header + payload).
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height,
                                  int qp,
                                  int levels = 1,
                                  int coder_mode = 1);

// Decode one frame.
// Returns row-major 8-bit mono of size width * height.
// Throws std::runtime_error on malformed bitstream.
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream);

// ------------------------------------------------------------------
// Rate control (CSV-driven)
// ------------------------------------------------------------------

// Select the highest QP whose measured bpp is still <= target_bpp.
// curve_name: "thermal" or "visible" (case-insensitive).
// Returns -1 if the curve cannot be loaded.
int select_qp_for_target_bpp(const std::string& curve_name, double target_bpp);

// ------------------------------------------------------------------
// Convenience
// ------------------------------------------------------------------

// Encode using a target bitrate instead of an explicit QP.
// Returns empty vector on failure.
inline std::vector<uint8_t> encode_frame_target_bpp(
        const std::vector<uint8_t>& pixels,
        int width, int height,
        double target_bpp,
        const std::string& curve_name = "thermal",
        int levels = 1,
        int coder_mode = 1)
{
    int qp = select_qp_for_target_bpp(curve_name, target_bpp);
    if (qp < 0) return {};
    return encode_frame(pixels, width, height, qp, levels, coder_mode);
}