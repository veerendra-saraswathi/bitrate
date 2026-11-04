#include "jpegls.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

struct jpegls_encoder {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t bit_depth = 8;
    uint8_t components = 1;
    int near = 0;
};

struct BitWriter {
    std::vector<uint8_t>& out;
    uint64_t acc = 0;
    int bits = 0;
    explicit BitWriter(std::vector<uint8_t>& o) : out(o) {}
    void put(uint32_t v, int n) {
        if (n <= 0) return;
        acc = (acc << n) | (v & ((1ULL << n) - 1));
        bits += n;
        while (bits >= 8) {
            bits -= 8;
            uint8_t b = (acc >> bits) & 0xFF;
            out.push_back(b);
            if (b == 0xFF) out.push_back(0x00);
        }
    }
    void flush() { if (bits > 0) put(0, 8 - bits); }
};

struct Ctx { int A = 4, B = 0, C = 0, N = 1; };

static int predict(int a, int b, int c) {
    if (c >= std::max(a, b)) return std::min(a, b);
    if (c <= std::min(a, b)) return std::max(a, b);
    return a + b - c;
}

static int qg(int d) {
    static const int T[4] = {0, 3, 7, 21}; // simplified thresholds for NEAR=0
    if (d <= -T[3]) return -4;
    if (d <= -T[2]) return -3;
    if (d <= -T[1]) return -2;
    if (d < 0) return -1;
    if (d == 0) return 0;
    if (d < T[1]) return 1;
    if (d < T[2]) return 2;
    if (d < T[3]) return 3;
    return 4;
}

static int get_k(int A, int N) {
    int k = 0;
    for (; (N << k) < A && k < 8; ++k) {}
    return k;
}

static void golomb(BitWriter& bw, int mapped, int k) {
    int unary = mapped >> k;
    int limit = 23; // LIMIT - Qbpp - 1 for 8-bit
    if (unary < limit) {
        bw.put(1, unary + 1);          // unary zeros + 1
        if (k) bw.put(mapped & ((1 << k) - 1), k);
    } else {
        bw.put(1, limit + 1);
        bw.put(mapped - 1, 8);
    }
}

jpegls_encoder* jpegls_encoder_create() { return new jpegls_encoder; }
void jpegls_encoder_destroy(jpegls_encoder* e) { delete e; }

int jpegls_encoder_set_frame_info(jpegls_encoder* e, uint32_t w, uint32_t h, uint8_t bd, uint8_t c) {
    if (!e || c != 1 || bd != 8) return -1;
    e->width = w; e->height = h; e->bit_depth = bd; e->components = c;
    return 0;
}
int jpegls_encoder_set_near(jpegls_encoder* e, int n) {
    if (!e || n != 0) return -1;
    return 0;
}

int jpegls_encoder_encode(jpegls_encoder* e, const void* samples, size_t ns, uint8_t* dest, size_t cap) {
    if (!e || !samples || !dest || ns != size_t(e->width)*e->height) return -1;
    const uint8_t* img = static_cast<const uint8_t*>(samples);
    int W = e->width, H = e->height;

    std::vector<uint8_t> stream;
    stream.reserve(ns / 2);

    // Markers
    stream.insert(stream.end(), {0xFF,0xD8});
    stream.insert(stream.end(), {0xFF,0xF7, 0x00,0x0B, 8,
                                 uint8_t(H>>8),uint8_t(H), uint8_t(W>>8),uint8_t(W),
                                 1,1,0x11,0});
    stream.insert(stream.end(), {0xFF,0xDA, 0x00,0x08, 1,1,0, 0,0,0});

    BitWriter bw(stream);
    std::vector<Ctx> ctx(365);
    std::vector<int> line(W+2, 0), prev(W+2, 0);

    for (int y = 0; y < H; ++y) {
        std::fill(line.begin(), line.end(), 0);
        for (int x = 0; x < W; ++x) {
            int Ra = line[x], Rb = prev[x+1], Rc = prev[x], Rd = prev[x+2];
            int Ix = img[y*W+x];
            int D1 = Rd-Rb, D2 = Rb-Rc, D3 = Rc-Ra;
            int q1 = qg(D1), q2 = qg(D2), q3 = qg(D3);
            int sgn = 1;
            if (q1<0 || (q1==0 && q2<0) || (q1==0 && q2==0 && q3<0)) {
                sgn = -1; q1=-q1; q2=-q2; q3=-q3;
            }
            int ci = (q1*9 + q2)*9 + q3; // reduced indexing
            if (ci >= 365) ci = 0;
            Ctx& c = ctx[ci];

            int Px = predict(Ra, Rb, Rc);
            int err = Ix - Px;
            if (sgn < 0) err = -err;
            err = (err + 128) & 255; if (err >= 128) err -= 256; // modulo

            int mapped = err >= 0 ? 2*err : -2*err-1;
            int k = get_k(c.A, c.N);
            golomb(bw, mapped, k);

            c.A += std::abs(err);
            if (c.N == 64) { c.A >>= 1; c.N >>= 1; }
            c.N++;

            line[x+1] = Ix;
        }
        prev.swap(line);
    }
    bw.flush();
    stream.insert(stream.end(), {0xFF,0xD9});

    if (stream.size() > cap) return -3;
    std::memcpy(dest, stream.data(), stream.size());
    return int(stream.size());
}
