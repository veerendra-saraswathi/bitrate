#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <limits>

// ---------- Mode 0 decoder ----------
static bool entropy_decode_legacy(const uint8_t* data, size_t n, size_t expected,
                                  std::vector<int32_t>& coeffs) {
    coeffs.clear();
    coeffs.reserve(expected);
    size_t pos = 0;
    while (pos < n && coeffs.size() < expected) {
        uint8_t marker = data[pos++];
        if (marker == 0x00 && pos < n) {
            uint8_t run = data[pos++];
            for (int k = 0; k < run && coeffs.size() < expected; ++k)
                coeffs.push_back(0);
        } else if (marker == 0x01 && pos + 5 <= n) {
            uint32_t u = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
            pos += 4;
            int sign = data[pos++];
            int32_t v = static_cast<int32_t>(u);
            if (sign) v = -v;
            coeffs.push_back(v);
        } else {
            return false;
        }
    }
    return coeffs.size() == expected && pos == n;
}

// ---------- Mode 1 decoder ----------
static bool read_varint(const uint8_t* data, size_t size, size_t& pos, uint32_t& value) {
    value = 0;
    for (unsigned shift = 0; shift <= 28; shift += 7) {
        if (pos >= size) return false;
        uint8_t byte = data[pos++];
        if (shift == 28 && (byte & 0xF0u) != 0) return false;
        value |= static_cast<uint32_t>(byte & 0x7Fu) << shift;
        if ((byte & 0x80u) == 0) return true;
    }
    return false;
}

static bool zigzag_decode(uint32_t value, int32_t& result) {
    if ((value & 1u) == 0) {
        result = static_cast<int32_t>(value >> 1);
        return true;
    }
    uint32_t magnitude = (value >> 1) + 1u;
    if (magnitude == 0x80000000u)
        result = std::numeric_limits<int32_t>::min();
    else
        result = -static_cast<int32_t>(magnitude);
    return true;
}

static bool entropy_decode_varint(const uint8_t* data, size_t size, size_t expected,
                                  std::vector<int32_t>& coeffs) {
    size_t pos = 0;
    coeffs.clear();
    coeffs.reserve(expected);
    while (coeffs.size() < expected) {
        uint32_t encoded = 0;
        if (!read_varint(data, size, pos, encoded)) return false;
        int32_t value = 0;
        if (!zigzag_decode(encoded, value)) return false;
        coeffs.push_back(value);
    }
    return pos == size;
}

// ---------- Public decoder ----------
std::vector<uint8_t> decode_frame(const std::vector<uint8_t>& bitstream) {
    BitstreamReader reader(bitstream);
    FrameHeader h = reader.read_header();

    int width = h.width;
    int height = h.height;
    int qp = h.qp;
    int levels = h.levels ? h.levels : 1;
    int TS = h.tile_size ? h.tile_size : 64;

    std::vector<uint8_t> out(width * height, 0);
    const size_t header_size = 25;
    if (bitstream.size() < header_size) return out;
    const uint8_t* payload = bitstream.data() + header_size;
    size_t payload_len = bitstream.size() - header_size;

    size_t p = 0;
    for (int ty = 0; ty < height; ty += TS) {
        for (int tx = 0; tx < width; tx += TS) {
            int tw = std::min(TS, width - tx);
            int th = std::min(TS, height - ty);
            size_t expected = static_cast<size_t>(tw) * th;

            if (p + 4 > payload_len) break;
            uint32_t len = payload[p] | (payload[p+1] << 8) | (payload[p+2] << 16) | (payload[p+3] << 24);
            p += 4;
            if (p + len > payload_len || len == 0) break;

            const uint8_t* tile_payload = payload + p;
            uint8_t coder_mode = tile_payload[0];
            const uint8_t* entropy_data = tile_payload + 1;
            size_t entropy_size = len - 1;
            p += len;

            std::vector<int32_t> coeffs;
            bool ok = false;
            if (coder_mode == 0)
                ok = entropy_decode_legacy(entropy_data, entropy_size, expected, coeffs);
            else if (coder_mode == 1)
                ok = entropy_decode_varint(entropy_data, entropy_size, expected, coeffs);
            else
                throw std::runtime_error("Unknown entropy coder mode");

            if (!ok)
                throw std::runtime_error("Invalid tile entropy data");

            dequantise(coeffs, qp);
            wavelet_5_3_inverse(coeffs, tw, th, levels);

            for (int y = 0; y < th; ++y)
                for (int x = 0; x < tw; ++x) {
                    int32_t v = coeffs[y * tw + x];
                    out[(ty + y) * width + (tx + x)] = static_cast<uint8_t>(std::max(0, std::min(v, 255)));
                }
        }
    }
    return out;
}
