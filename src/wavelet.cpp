#include "wavelet.h"
#include <algorithm>
#include <vector>
#include <cstdint>

// ---------- 1-D lifting (unchanged) ----------
static void lift_1d_forward(int32_t* s, int n) {
    for (int i = 1; i < n; i += 2) {
        int32_t left = s[i - 1];
        int32_t right = (i + 1 < n) ? s[i + 1] : left;
        s[i] -= (left + right) >> 1;
    }
    for (int i = 0; i < n; i += 2) {
        int32_t prev = (i > 0) ? s[i - 1] : s[i + 1];
        int32_t next = (i + 1 < n) ? s[i + 1] : prev;
        s[i] += (prev + next + 2) >> 2;
    }
}

static void lift_1d_inverse(int32_t* s, int n) {
    for (int i = 0; i < n; i += 2) {
        int32_t prev = (i > 0) ? s[i - 1] : s[i + 1];
        int32_t next = (i + 1 < n) ? s[i + 1] : prev;
        s[i] -= (prev + next + 2) >> 2;
    }
    for (int i = 1; i < n; i += 2) {
        int32_t left = s[i - 1];
        int32_t right = (i + 1 < n) ? s[i + 1] : left;
        s[i] += (left + right) >> 1;
    }
}

// ---------- Single-level 2-D (baseline) ----------
static void wavelet_2d_forward(std::vector<int32_t>& tile, int width, int height) {
    for (int y = 0; y < height; ++y)
        lift_1d_forward(tile.data() + y * width, width);

    std::vector<int32_t> col(height);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y)
            col[y] = tile[y * width + x];
        lift_1d_forward(col.data(), height);
        for (int y = 0; y < height; ++y)
            tile[y * width + x] = col[y];
    }
}

static void wavelet_2d_inverse(std::vector<int32_t>& tile, int width, int height) {
    std::vector<int32_t> col(height);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y)
            col[y] = tile[y * width + x];
        lift_1d_inverse(col.data(), height);
        for (int y = 0; y < height; ++y)
            tile[y * width + x] = col[y];
    }
    for (int y = 0; y < height; ++y)
        lift_1d_inverse(tile.data() + y * width, width);
}

// ---------- Multi-level cascading on LL ----------
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height, int levels) {
    if (levels < 1) levels = 1;
    int w = width, h = height;
    for (int L = 0; L < levels; ++L) {
        if (w < 2 || h < 2) break;
        // Extract LL region into a temporary buffer for in-place transform
        std::vector<int32_t> ll(w * h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                ll[y * w + x] = tile[y * width + x];

        wavelet_2d_forward(ll, w, h);

        // Write back only the LL region
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                tile[y * width + x] = ll[y * w + x];

        w >>= 1;
        h >>= 1;
    }
}

void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height, int levels) {
    if (levels < 1) levels = 1;
    // Compute the dimensions at each level (deepest first)
    std::vector<std::pair<int,int>> dims;
    int w = width, h = height;
    for (int L = 0; L < levels; ++L) {
        if (w < 2 || h < 2) break;
        dims.push_back({w, h});
        w >>= 1;
        h >>= 1;
    }

    // Inverse from deepest to shallowest
    for (int i = static_cast<int>(dims.size()) - 1; i >= 0; --i) {
        w = dims[i].first;
        h = dims[i].second;
        std::vector<int32_t> ll(w * h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                ll[y * w + x] = tile[y * width + x];

        wavelet_2d_inverse(ll, w, h);

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                tile[y * width + x] = ll[y * w + x];
    }
}

// ---------- Quantisation (unchanged) ----------
void quantise(std::vector<int32_t>& coeffs, int qp) {
    if (qp <= 1) return;
    for (auto& c : coeffs) {
        int32_t sign = (c < 0) ? -1 : 1;
        c = sign * (std::abs(c) / qp);
    }
}

void dequantise(std::vector<int32_t>& coeffs, int qp) {
    if (qp <= 1) return;
    for (auto& c : coeffs)
        c *= qp;
}

// Compatibility wrappers so existing 3-argument calls still link
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height) {
    wavelet_5_3_forward(tile, width, height, 1);
}

void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height) {
    wavelet_5_3_inverse(tile, width, height, 1);
}
