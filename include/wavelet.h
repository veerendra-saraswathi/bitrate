#pragma once
#include <vector>
#include <cstdint>

// Reversible 5/3 lifting wavelet (integer)
void wavelet_5_3_forward(std::vector<int32_t>& tile, int width, int height);
void wavelet_5_3_inverse(std::vector<int32_t>& tile, int width, int height);

// Simple uniform quantisation
void quantise(std::vector<int32_t>& coeffs, int qp);
void dequantise(std::vector<int32_t>& coeffs, int qp);