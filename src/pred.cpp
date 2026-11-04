#include "pred.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
constexpr int BS = 8;
constexpr int TILE = 32;

uint32_t crc32(const uint8_t* d, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) {
        c ^= d[i];
        for (int b = 0; b < 8; ++b) c = (c >> 1) ^ (0xEDB88320u & -(c & 1u));
    }
    return ~c;
}
uint32_t zz_enc(int32_t v) {
    return v >= 0 ? (uint32_t)v * 2u : (uint32_t)(-(int64_t)v) * 2u - 1u;
}
int32_t zz_dec(uint32_t v) {
    return (v & 1) ? -(int32_t)((v >> 1) + 1) : (int32_t)(v >> 1);
}
void wv(uint32_t v, std::vector<uint8_t>& o) {
    while (v >= 0x80u) { o.push_back((v & 0x7f) | 0x80); v >>= 7; }
    o.push_back((uint8_t)v);
}
bool rv(const uint8_t* d, size_t n, size_t& p, uint32_t& v) {
    v = 0;
    for (unsigned s = 0; s <= 28; s += 7) {
        if (p >= n) return false;
        uint8_t b = d[p++];
        v |= (uint32_t)(b & 0x7f) << s;
        if (!(b & 0x80)) return true;
    }
    return false;
}
inline int quant(int r, int qp) {
    if (qp <= 1) return r;
    int thr = qp;
    if (std::abs(r) < thr) return 0;
    return (r >= 0) ? (r + qp / 2) / qp : -((-r + qp / 2) / qp);
}
inline int dequant(int q, int qp) { return qp <= 1 ? q : q * qp; }

inline int median3(int a, int b, int c) {
    if (a > b) std::swap(a, b);
    if (b > c) std::swap(b, c);
    if (a > b) std::swap(a, b);
    return b;
}

void encode_block(const uint8_t* px, int w, int x0, int y0, int qp,
                  std::vector<uint8_t>& blk) {
    int res[64];
    int i = 0;
    for (int y = 0; y < BS; ++y) {
        for (int x = 0; x < BS; ++x, ++i) {
            int left  = (x > 0) ? px[(y0 + y) * w + (x0 + x - 1)] : 128;
            int above = (y > 0) ? px[(y0 + y - 1) * w + (x0 + x)] : 128;
            int al    = (x > 0 && y > 0) ? px[(y0 + y - 1) * w + (x0 + x - 1)] : 128;
            int pred  = median3(left, above, al);
            int r = (int)px[(y0 + y) * w + (x0 + x)] - pred;
            res[i] = quant(r, qp);
        }
    }
    int last = -1;
    for (int k = 0; k < 64; ++k) if (res[k]) last = k;
    int run = 0;
    for (int k = 0; k <= last; ++k) {
        if (res[k] == 0) ++run;
        else {
            wv((uint32_t)run, blk);
            wv(zz_enc(res[k]), blk);
            run = 0;
        }
    }
    int rem = 63 - last;
    if (rem > 0) wv((uint32_t)rem, blk);
}

void decode_block(const uint8_t* d, size_t len, int w, int x0, int y0, int qp,
                  uint8_t* out) {
    int res[64] = {0};
    size_t p = 0;
    int i = 0;
    while (i < 64 && p < len) {
        uint32_t run = 0;
        if (!rv(d, len, p, run)) break;
        i += (int)run;
        if (i >= 64) break;
        uint32_t e = 0;
        if (!rv(d, len, p, e)) break;
        if (i < 64) res[i] = zz_dec(e);
        ++i;
    }
    i = 0;
    for (int y = 0; y < BS; ++y) {
        for (int x = 0; x < BS; ++x, ++i) {
            int left  = (x > 0) ? out[(y0 + y) * w + (x0 + x - 1)] : 128;
            int above = (y > 0) ? out[(y0 + y - 1) * w + (x0 + x)] : 128;
            int al    = (x > 0 && y > 0) ? out[(y0 + y - 1) * w + (x0 + x - 1)] : 128;
            int pred  = median3(left, above, al);
            int v = pred + dequant(res[i], qp);
            out[(y0 + y) * w + (x0 + x)] = (uint8_t)std::clamp(v, 0, 255);
        }
    }
}
}  // namespace

std::vector<uint8_t> pred_encode_frame(const std::vector<uint8_t>& px, int w, int h, int qp) {
    if (w % TILE || h % TILE) throw std::invalid_argument("multiple of 32 required");
    std::vector<uint8_t> pay;
    for (int ty = 0; ty < h; ty += TILE)
        for (int tx = 0; tx < w; tx += TILE) {
            std::vector<uint8_t> tile;
            for (int by = 0; by < TILE; by += BS)
                for (int bx = 0; bx < TILE; bx += BS) {
                    std::vector<uint8_t> blk;
                    encode_block(px.data(), w, tx + bx, ty + by, qp, blk);
                    uint16_t bl = (uint16_t)blk.size();
                    tile.push_back(bl & 0xff);
                    tile.push_back(bl >> 8);
                    tile.insert(tile.end(), blk.begin(), blk.end());
                }
            uint32_t crc = crc32(tile.data(), tile.size());
            uint32_t len = (uint32_t)tile.size();
            pay.push_back(len & 0xff); pay.push_back(len >> 8);
            pay.push_back(len >> 16); pay.push_back(len >> 24);
            pay.insert(pay.end(), tile.begin(), tile.end());
            pay.push_back(crc & 0xff); pay.push_back(crc >> 8);
            pay.push_back(crc >> 16); pay.push_back(crc >> 24);
        }
    return pay;
}

std::vector<uint8_t> pred_decode_frame(const std::vector<uint8_t>& pay, int w, int h, int qp) {
    std::vector<uint8_t> out(w * h, 0);
    size_t pos = 0;
    for (int ty = 0; ty < h && pos + 8 <= pay.size(); ty += TILE)
        for (int tx = 0; tx < w && pos + 8 <= pay.size(); tx += TILE) {
            uint32_t len = pay[pos] | (pay[pos+1]<<8) | (pay[pos+2]<<16) | (pay[pos+3]<<24);
            pos += 4;
            if (pos + len + 4 > pay.size()) return out;
            const uint8_t* td = pay.data() + pos;
            uint32_t sc = pay[pos+len] | (pay[pos+len+1]<<8) | (pay[pos+len+2]<<16) | (pay[pos+len+3]<<24);
            if (crc32(td, len) != sc) { pos += len + 4; continue; }
            pos += len + 4;
            size_t tp = 0;
            for (int by = 0; by < TILE; by += BS)
                for (int bx = 0; bx < TILE; bx += BS) {
                    if (tp + 2 > len) break;
                    uint16_t bl = td[tp] | (td[tp+1]<<8); tp += 2;
                    if (tp + bl > len) break;
                    decode_block(td + tp, bl, w, tx + bx, ty + by, qp, out.data());
                    tp += bl;
                }
        }
    return out;
}
