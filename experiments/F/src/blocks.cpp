#include "blocks.h"
#include "../include/f_codec.h"
#include "rice.h"
#include "rans.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace f {

static void write_u16(std::vector<uint8_t>& o, uint16_t v) {
  o.push_back(v & 0xff); o.push_back((v >> 8) & 0xff);
}
static void write_u32(std::vector<uint8_t>& o, uint32_t v) {
  for (int i = 0; i < 4; ++i) o.push_back((v >> (8 * i)) & 0xff);
}
static uint16_t read_u16(const uint8_t*& p) {
  uint16_t v = p[0] | (p[1] << 8); p += 2; return v;
}
static uint32_t read_u32(const uint8_t*& p) {
  uint32_t v = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24); p += 4; return v;
}

bool encode_subband_blocks(const int16_t* coeffs, int width, int height,
                           int levels, int block_size,
                           std::vector<uint8_t>& out,
                           EncodeStats* stats,
                           std::vector<SubbandStat>* sb_stats) {
  out.clear();
  out.push_back('F'); out.push_back('5'); out.push_back('3'); out.push_back('B');
  write_u16(out, width); write_u16(out, height);
  out.push_back(static_cast<uint8_t>(levels));
  write_u16(out, block_size);

  auto subbands = list_subbands(width, height, levels);
  size_t payload = 0;
  if (sb_stats) sb_stats->clear();

  for (const auto& sb : subbands) {
    SubbandStat sst{};
    sst.x = sb.x; sst.y = sb.y; sst.w = sb.w; sst.h = sb.h;
    sst.level = sb.level; sst.type = sb.type;
    sst.cmin = 32767; sst.cmax = -32768;
    int zeros = 0, total = 0;
    double ksum = 0;
    int blocks = 0;
    size_t sb_payload = 0;

    for (int by = 0; by < sb.h; by += block_size) {
      for (int bx = 0; bx < sb.w; bx += block_size) {
        int bw = std::min(block_size, sb.w - bx);
        int bh = std::min(block_size, sb.h - by);
        std::vector<int16_t> block(bw * bh);
        for (int y = 0; y < bh; ++y)
          for (int x = 0; x < bw; ++x) {
            int16_t c = coeffs[(sb.y + by + y) * width + (sb.x + bx + x)];
            block[y * bw + x] = c;
            if (c < sst.cmin) sst.cmin = c;
            if (c > sst.cmax) sst.cmax = c;
            if (c == 0) ++zeros;
            ++total;
          }

        uint8_t k = 0;
        auto pl = rice_encode_ctx(block.data(), block.size(), bw, bh, k);
        // rANS path disabled until independent tests pass
        // auto pl_rans = rans_encode_auto(...);
        ksum += k;
        ++blocks;
        sb_payload += pl.size();

        write_u16(out, sb.x + bx); write_u16(out, sb.y + by);
        write_u16(out, bw); write_u16(out, bh);
        out.push_back(k);
        write_u32(out, static_cast<uint32_t>(pl.size()));
        out.insert(out.end(), pl.begin(), pl.end());
        payload += pl.size();
      }
    }
    sst.zero_frac = total ? static_cast<double>(zeros) / total : 0;
    sst.avg_k = blocks ? ksum / blocks : 0;
    sst.payload_bytes = sb_payload;
    sst.blocks = blocks;
    if (sb_stats) sb_stats->push_back(sst);
  }

  if (stats) {
    stats->payload_bytes = payload;
    stats->total_bytes = out.size();
    stats->header_bytes = out.size() - payload;
  }
  return true;
}

bool decode_subband_blocks(const uint8_t* in, size_t len,
                           int width, int height, int levels, int block_size,
                           std::vector<int16_t>& coeffs) {
  if (len < 12) return false;
  const uint8_t* p = in;
  if (p[0]!='F' || p[1]!='5' || p[2]!='3' || p[3]!='B') return false;
  p += 4;
  int w = read_u16(p), h = read_u16(p);
  int lev = *p++;
  int bs = read_u16(p);
  if (w != width || h != height || lev != levels || bs != block_size) return false;

  coeffs.assign(width * height, 0);
  const uint8_t* end = in + len;
  while (p + 13 <= end) {
    int bx = read_u16(p), by = read_u16(p);
    int bw = read_u16(p), bh = read_u16(p);
    uint8_t k = *p++;
    uint32_t plen = read_u32(p);
    if (p + plen > end) return false;
    std::vector<int16_t> block;
    if (!rice_decode(p, plen, bw * bh, k, block)) return false;
    p += plen;
    for (int y = 0; y < bh; ++y)
      for (int x = 0; x < bw; ++x)
        coeffs[(by + y) * width + (bx + x)] = block[y * bw + x];
  }
  return true;
}

} // namespace f
