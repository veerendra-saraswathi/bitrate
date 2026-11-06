#include "../include/f_codec.h"
#include "wavelet53.h"
#include "blocks.h"
#include <chrono>
#include <vector>
#include <algorithm>

namespace f {

static double ms_since(std::chrono::high_resolution_clock::time_point t0) {
  auto t1 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

bool encode_frame(const uint8_t* src, int width, int height,
                  int levels, int block_size,
                  std::vector<uint8_t>& bitstream,
                  EncodeStats* stats,
                  std::vector<SubbandStat>* sb_stats) {
  auto t_all = std::chrono::high_resolution_clock::now();

  std::vector<int16_t> frame(width * height);
  for (int i = 0; i < width * height; ++i)
    frame[i] = static_cast<int16_t>(src[i]);

  auto t0 = std::chrono::high_resolution_clock::now();
  dwt53_forward_frame(frame, width, height, levels);
  double t_transform = ms_since(t0);

  EncodeStats local;
  t0 = std::chrono::high_resolution_clock::now();
  bool ok = encode_subband_blocks(frame.data(), width, height, levels, block_size,
                                  bitstream, &local, sb_stats);
  double t_rest = ms_since(t0);

  if (stats) {
    *stats = local;
    stats->transform_ms = t_transform;
    stats->analysis_ms = t_rest * 0.3;   // rough split; refined inside blocks if needed
    stats->payload_ms = t_rest * 0.7;
    stats->total_ms = ms_since(t_all);
  }
  return ok;
}

bool decode_frame(const uint8_t* bitstream, size_t len,
                  int width, int height, int levels, int block_size,
                  std::vector<uint8_t>& dst) {
  std::vector<int16_t> frame;
  if (!decode_subband_blocks(bitstream, len, width, height, levels, block_size, frame))
    return false;
  dwt53_inverse_frame(frame, width, height, levels);
  dst.resize(width * height);
  for (int i = 0; i < width * height; ++i) {
    int v = frame[i];
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    dst[i] = static_cast<uint8_t>(v);
  }
  return true;
}

bool encode_frame_u16(const uint16_t* src, int width, int height,
                      int levels, int block_size,
                      std::vector<uint8_t>& bitstream) {
  std::vector<int16_t> frame(width * height);
  for (int i = 0; i < width * height; ++i)
    frame[i] = static_cast<int16_t>(src[i] - 32768); // center for signed
  dwt53_forward_frame(frame, width, height, levels);
  return encode_subband_blocks(frame.data(), width, height, levels, block_size,
                               bitstream, nullptr, nullptr);
}

bool decode_frame_u16(const uint8_t* bitstream, size_t len,
                      int width, int height, int levels, int block_size,
                      std::vector<uint16_t>& dst) {
  std::vector<int16_t> frame;
  if (!decode_subband_blocks(bitstream, len, width, height, levels, block_size, frame))
    return false;
  dwt53_inverse_frame(frame, width, height, levels);
  dst.resize(width * height);
  for (int i = 0; i < width * height; ++i) {
    int v = frame[i] + 32768;
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst[i] = static_cast<uint16_t>(v);
  }
  return true;
}

} // namespace f
