#include "../include/f_lossy.h"
#include "../include/f_codec.h"
#include "wavelet53.h"
#include "blocks.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace f {

struct QuantStats {
  int total_coeffs = 0;
  int changed = 0;
  int zeros_before = 0;
  int zeros_after = 0;
};

bool encode_lossy_frame_u16(const uint16_t* src, int width, int height,
                            int quant_step, size_t budget_bytes,
                            LossyResult& result) {
  auto t0 = std::chrono::high_resolution_clock::now();

  std::vector<int16_t> frame(width * height);
  for (int i = 0; i < width * height; ++i)
    frame[i] = static_cast<int16_t>(src[i] - 32768);

  dwt53_forward_frame(frame, width, height, 1);

  QuantStats qs;
  qs.total_coeffs = width * height;
  for (auto c : frame) if (c == 0) ++qs.zeros_before;

  if (quant_step < 1) quant_step = 1;
  for (auto& c : frame) {
    int16_t q = static_cast<int16_t>((c / quant_step) * quant_step);
    if (q != c) ++qs.changed;
    c = q;
  }
  for (auto c : frame) if (c == 0) ++qs.zeros_after;

  std::vector<uint8_t> bs;
  bool enc_ok = encode_subband_blocks(frame.data(), width, height, 1, 32, bs, nullptr, nullptr);

  std::vector<uint8_t> final;
  final.push_back('F'); final.push_back('Q');
  final.push_back(static_cast<uint8_t>(quant_step & 0xff));
  final.push_back(static_cast<uint8_t>((quant_step >> 8) & 0xff));
  if (enc_ok) final.insert(final.end(), bs.begin(), bs.end());

  result.bytes = final.size();
  result.budget_met = enc_ok && (final.size() <= budget_bytes);
  result.quant_step = quant_step;
  result.bitstream = final;
  result.encode_ms = std::chrono::duration<double, std::milli>(
      std::chrono::high_resolution_clock::now() - t0).count();

  // Always attempt decode for verification
  std::vector<uint16_t> rec;
  bool dec_ok = false;
  if (enc_ok)
    dec_ok = decode_lossy_frame_u16(final.data(), final.size(), width, height, rec);

  result.budget_met = result.budget_met && dec_ok && (rec.size() == static_cast<size_t>(width * height));

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
    result.rmse = -1;
    result.max_err = -1;
    result.psnr = -1;
  }

  // Stash quant stats in unused fields for the bench to print (temporary)
  // (proper struct extension can come later)
  std::cout << "  [quant] total=" << qs.total_coeffs
            << " changed=" << qs.changed
            << " zeros_before=" << qs.zeros_before
            << " zeros_after=" << qs.zeros_after
            << " enc_ok=" << enc_ok
            << " dec_ok=" << dec_ok
            << " rec_samples=" << rec.size() << "\n";

  return result.budget_met;
}

bool decode_lossy_frame_u16(const uint8_t* bitstream, size_t len,
                            int width, int height,
                            std::vector<uint16_t>& dst) {
  if (len < 4 || bitstream[0] != 'F' || bitstream[1] != 'Q') return false;
  const uint8_t* payload = bitstream + 4;
  size_t plen = len - 4;
  std::vector<int16_t> frame;
  if (!decode_subband_blocks(payload, plen, width, height, 1, 32, frame))
    return false;
  dwt53_inverse_frame(frame, width, height, 1);
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
