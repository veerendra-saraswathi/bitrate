#pragma once
#include <vector>
#include <cstdint>

// 4-argument versions (cascading)
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height, int levels);
void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height, int levels);

// 3-argument wrappers (level 1)
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height);
void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height);

void quantise(std::vector<int32_t>& coeffs, int qp);
void dequantise(std::vector<int32_t>& coeffs, int qp);
