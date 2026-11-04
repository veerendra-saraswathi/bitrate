#pragma once
#include <cstdint>
#include <vector>

std::vector<uint8_t> pred_encode_frame(const std::vector<uint8_t>& pixels,
                                       int width, int height, int qp);

std::vector<uint8_t> pred_decode_frame(const std::vector<uint8_t>& payload,
                                       int width, int height, int qp);
