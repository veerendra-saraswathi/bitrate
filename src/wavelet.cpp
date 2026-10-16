#include "wavelet.h"

// Reversible 5/3 lifting (integer, perfect reconstruction when qp=1)
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height) {
    // Horizontal pass
    for (int y = 0; y < height; ++y) {
        for (int x = 1; x < width - 1; x += 2) {
            tile[y * width + x] -= (tile[y * width + x - 1] + tile[y * width + x + 1]) >> 1;
        }
        for (int x = 2; x < width; x += 2) {
            tile[y * width + x] += (tile[y * width + x - 1] + tile[y * width + x + 1]) >> 2;
        }
    }
    // Vertical pass
    for (int x = 0; x < width; ++x) {
        for (int y = 1; y < height - 1; y += 2) {
            tile[y * width + x] -= (tile[(y - 1) * width + x] + tile[(y + 1) * width + x]) >> 1;
        }
        for (int y = 2; y < height; y += 2) {
            tile[y * width + x] += (tile[(y - 1) * width + x] + tile[(y + 1) * width + x]) >> 2;
        }
    }
}

void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height) {
    // Inverse vertical
    for (int x = 0; x < width; ++x) {
        for (int y = 2; y < height; y += 2) {
            tile[y * width + x] -= (tile[(y - 1) * width + x] + tile[(y + 1) * width + x]) >> 2;
        }
        for (int y = 1; y < height - 1; y += 2) {
            tile[y * width + x] += (tile[(y - 1) * width + x] + tile[(y + 1) * width + x]) >> 1;
        }
    }
    // Inverse horizontal
    for (int y = 0; y < height; ++y) {
        for (int x = 2; x < width; x += 2) {
            tile[y * width + x] -= (tile[y * width + x - 1] + tile[y * width + x + 1]) >> 2;
        }
        for (int x = 1; x < width - 1; x += 2) {
            tile[y * width + x] += (tile[y * width + x - 1] + tile[y * width + x + 1]) >> 1;
        }
    }
}

void quantise(std::vector<int32_t>& coeffs, int qp) {
    if (qp <= 1) return;
    for (auto& c : coeffs) c /= qp;
}

void dequantise(std::vector<int32_t>& coeffs, int qp) {
    if (qp <= 1) return;
    for (auto& c : coeffs) c *= qp;
}