#pragma once

#include <cstdint>
#include <vector>
#include <cstddef>
#include <string>

namespace f {

struct EncodeStats {
  size_t payload_bytes = 0;
  size_t header_bytes = 0;
  size_t total_bytes = 0;
  double transform_ms = 0;
  double analysis_ms = 0;
  double payload_ms = 0;
  double total_ms = 0;
  bool bit_exact = false;
  int max_err = 0;
};

struct SubbandStat {
  int x, y, w, h;
  int level;
  char type;
  int16_t cmin, cmax;
  double zero_frac;
  double avg_k;
  size_t payload_bytes;
  int blocks;
};

// levels fixed to 1 for baseline measurements unless overridden.
bool encode_frame(const uint8_t* src, int width, int height,
                  int levels, int block_size,
                  std::vector<uint8_t>& bitstream,
                  EncodeStats* stats = nullptr,
                  std::vector<SubbandStat>* sb_stats = nullptr);

bool decode_frame(const uint8_t* bitstream, size_t len,
                  int width, int height, int levels, int block_size,
                  std::vector<uint8_t>& dst);

// 16-bit path for native bit-depth tests
bool encode_frame_u16(const uint16_t* src, int width, int height,
                      int levels, int block_size,
                      std::vector<uint8_t>& bitstream);
bool decode_frame_u16(const uint8_t* bitstream, size_t len,
                      int width, int height, int levels, int block_size,
                      std::vector<uint16_t>& dst);

} // namespace f
