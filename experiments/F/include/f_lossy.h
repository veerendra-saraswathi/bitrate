#pragma once

#include <cstdint>
#include <vector>

namespace f {

struct LossyResult {
  std::vector<uint8_t> bitstream;
  double encode_ms = 0;
  double psnr = 0;          // peak = 16383 for 14-bit, or 255 for 8-bit
  double rmse = 0;
  int max_err = 0;
  int quant_step = 1;
  bool budget_met = false;
  size_t bytes = 0;
};

// Separate lossy path. Does not alter the lossless encoder.
// quant_step is applied to wavelet coefficients before Rice.
// Budget is hard: if the coded size exceeds budget_bytes the call fails.
bool encode_lossy_frame_u16(const uint16_t* src, int width, int height,
                            int quant_step, size_t budget_bytes,
                            LossyResult& result);

bool decode_lossy_frame_u16(const uint8_t* bitstream, size_t len,
                            int width, int height,
                            std::vector<uint16_t>& dst);

} // namespace f
