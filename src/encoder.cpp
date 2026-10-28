#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>

// ---------- Mode 0: existing zero-run ----------
static void entropy_encode(const std::vector<int32_t>& coeffs, std::vector<uint8_t>& out) {
    size_t i = 0;
    while (i < coeffs.size()) {
        if (coeffs[i] == 0) {
            size_t run = 0;
            while (i < coeffs.size() && coeffs[i] == 0 && run < 255) {
                ++run; ++i;
            }
            out.push_back(0x00);
            out.push_back(static_cast<uint8_t>(run));
        } else {
            int32_t v = coeffs[i];
            out.push_back(0x01);
            uint32_t u = static_cast<uint32_t>(std::abs(v));
            out.push_back(static_cast<uint8_t>(u & 0xFF));
            out.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
            out.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
            out.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
            out.push_back(v < 0 ? 1 : 0);
            ++i;
        }
    }
}

// ---------- Mode 1: zigzag + varint with zero-run ----------
static uint32_t zigzag_encode(int32_t value) {
    if (value >= 0) return static_cast<uint32_t>(value) * 2u;
    uint32_t magnitude = static_cast<uint32_t>(-static_cast<int64_t>(value));
    return magnitude * 2u - 1u;
}

static void write_varint(uint32_t value, std::vector<uint8_t>& out) {
    while (value >= 0x80u) {
        out.push_back(static_cast<uint8_t>((value & 0x7Fu) | 0x80u));
        value >>= 7;
    }
    out.push_back(static_cast<uint8_t>(value));
}

static void entropy_encode_varint(const std::vector<int32_t>& coeffs, std::vector<uint8_t>& out) {
    size_t i = 0;
    const size_t n = coeffs.size();
    while (i < n) {
        size_t run = 0;
        while (i < n && coeffs[i] == 0 && run < 0xFFFF) {
            ++run;
            ++i;
        }
        write_varint(static_cast<uint32_t>(run), out);
        if (i >= n) break;
        write_varint(zigzag_encode(coeffs[i]), out);
        ++i;
    }
}

// ---------- Public API ----------
// Note: levels=1 recommended as default (best rate-distortion on measured KAIST data)
std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels, int coder_mode);

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels) {
    return encode_frame(pixels, width, height, qp, levels, 1);  // default mode 1
}

std::vector<uint8_t> encode_frame(const std::vector<uint8_t>& pixels,
                                  int width, int height, int qp,
                                  int levels, int coder_mode) {
    if (coder_mode != 0 && coder_mode != 1)
        throw std::invalid_argument("Unsupported entropy coder mode");

    FrameHeader h;
    h.width       = static_cast<uint16_t>(width);
    h.height      = static_cast<uint16_t>(height);
    h.qp          = static_cast<uint8_t>(qp);
    h.levels      = static_cast<uint8_t>(levels);
    h.coder_mode  = static_cast<uint8_t>(coder_mode);
    h.tile_size   = 64;
    h.version     = 0x0002;
    h.header_size = 64;
    h.bit_depth   = 8;
    h.channels    = 1;
    if (qp == 1) h.flags |= 0x01;   // perfect-reconstruction flag

    BitstreamWriter writer;
    writer.write_header(h);

    const int TS = h.tile_size;
    std::vector<uint8_t> payload;

    for (int ty = 0; ty < height; ty += TS) {
        for (int tx = 0; tx < width; tx += TS) {
            int tw = std::min(TS, width - tx);
            int th = std::min(TS, height - ty);

            std::vector<int32_t> tile(tw * th);
            for (int y = 0; y < th; ++y)
                for (int x = 0; x < tw; ++x)
                    tile[y * tw + x] = pixels[(ty + y) * width + (tx + x)];

            wavelet_5_3_forward(tile, tw, th, levels);
            quantise(tile, qp);

            std::vector<uint8_t> coded;
            coded.push_back(static_cast<uint8_t>(coder_mode));

            if (coder_mode == 0)
                entropy_encode(tile, coded);
            else
                entropy_encode_varint(tile, coded);

            uint32_t len = static_cast<uint32_t>(coded.size());
            payload.push_back(len & 0xFF);
            payload.push_back((len >> 8) & 0xFF);
            payload.push_back((len >> 16) & 0xFF);
            payload.push_back((len >> 24) & 0xFF);
            payload.insert(payload.end(), coded.begin(), coded.end());
        }
    }

    writer.write_bytes(payload.data(), payload.size());
    return writer.data();
}
