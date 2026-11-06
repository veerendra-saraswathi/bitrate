#include "../include/f_lossy.h"
#include "../include/f_codec.h"
#include "wavelet53_i32.h"
#include "rice_i32.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

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

bool encode_lossy_frame_u16(const uint16_t* src, int width, int height,
                            int quant_step, size_t budget_bytes,
                            LossyResult& result) {
  auto t0 = std::chrono::high_resolution_clock::now();

  std::vector<int32_t> frame(width * height);
  for (int i = 0; i < width * height; ++i)
    frame[i] = static_cast<int32_t>(src[i]) - 32768;

  dwt53_forward_frame_i32(frame, width, height, 1);

  if (quant_step < 1) quant_step = 1;
  int changed = 0;
  for (auto& c : frame) {
    int32_t q = (c / quant_step) * quant_step;
    if (q != c) ++changed;
    c = q;
  }

  // Full int32 Rice path — no narrowing
  std::vector<uint8_t> bs;
  bs.push_back('F'); bs.push_back('Q'); bs.push_back('3'); bs.push_back('2');
  write_u16(bs, width); write_u16(bs, height);
  write_u16(bs, quant_step);

  const int block_size = 32;
  for (int by = 0; by < height; by += block_size) {
    for (int bx = 0; bx < width; bx += block_size) {
      int bw = std::min(block_size, width - bx);
      int bh = std::min(block_size, height - by);
      std::vector<int32_t> block(bw * bh);
      for (int y = 0; y < bh; ++y)
        for (int x = 0; x < bw; ++x)
          block[y * bw + x] = frame[(by + y) * width + (bx + x)];

      uint8_t k = 0;
      auto pl = rice_encode_i32_ctx(block.data(), block.size(), bw, bh, k);
      write_u16(bs, bx); write_u16(bs, by);
      write_u16(bs, bw); write_u16(bs, bh);
      bs.push_back(k);
      write_u32(bs, static_cast<uint32_t>(pl.size()));
      bs.insert(bs.end(), pl.begin(), pl.end());
    }
  }

  result.bytes = bs.size();
  result.quant_step = quant_step;
  result.bitstream = bs;
  result.encode_ms = std::chrono::duration<double, std::milli>(
      std::chrono::high_resolution_clock::now() - t0).count();

  std::vector<uint16_t> rec;
  bool dec_ok = decode_lossy_frame_u16(bs.data(), bs.size(), width, height, rec);
  result.budget_met = dec_ok && (bs.size() <= budget_bytes) &&
                      (rec.size() == static_cast<size_t>(width * height));

  if (dec_ok && rec.size() == static_cast<size_t>(width * height)) {
    double mse = 0;
    int maxe = 0;
    for (int i = 0; i < width * height; ++i) {
      int d = int(src[i]) - int(rec[i]);
      mse += double(d) * d;
      maxe = std::max(maxe, std::abs(d));
    }
    mse /= (width * height);
    result.rmse = std::sqrt(mse);
    result.max_err = maxe;
    const double peak = 16383.0;
    result.psnr = (mse == 0) ? 99.0 : 10.0 * std::log10(peak * peak / mse);
  } else {
    result.rmse = -1; result.max_err = -1; result.psnr = -1;
  }

  std::cout << "  [i32] changed=" << changed << " dec_ok=" << dec_ok
            << " rec=" << rec.size() << "\n";
  return result.budget_met;
}

bool decode_lossy_frame_u16(const uint8_t* bitstream, size_t len,
                            int width, int height,
                            std::vector<uint16_t>& dst) {
  if (len < 10 || bitstream[0] != 'F' || bitstream[1] != 'Q' ||
      bitstream[2] != '3' || bitstream[3] != '2') return false;
  const uint8_t* p = bitstream + 4;
  int w = read_u16(p), h = read_u16(p);
  int qstep = read_u16(p);
  if (w != width || h != height) return false;

  std::vector<int32_t> frame(width * height, 0);
  const uint8_t* end = bitstream + len;
  while (p + 13 <= end) {
    int bx = read_u16(p), by = read_u16(p);
    int bw = read_u16(p), bh = read_u16(p);
    uint8_t k = *p++;
    uint32_t plen = read_u32(p);
    if (p + plen > end) return false;
    std::vector<int32_t> block;
    if (!rice_decode_i32(p, plen, bw * bh, k, block)) return false;
    p += plen;
    for (int y = 0; y < bh; ++y)
      for (int x = 0; x < bw; ++x)
        frame[(by + y) * width + (bx + x)] = block[y * bw + x];
  }

  dwt53_inverse_frame_i32(frame, width, height, 1);

  dst.resize(width * height);
  for (int i = 0; i < width * height; ++i) {
    int32_t v = frame[i] + 32768;
    if (v < 0) v = 0;
    if (v > 65535) v = 65535;
    dst[i] = static_cast<uint16_t>(v);
  }
  (void)qstep;
  return true;
}

} // namespace f
