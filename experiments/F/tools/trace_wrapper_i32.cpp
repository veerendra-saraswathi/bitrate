#include "../include/f_codec.h"
#include "../src/wavelet53_i32.h"
#include "../src/blocks.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

int main() {
  const int W = 640, H = 512;
  const int qstep = 4;

  std::ifstream f("../../../data/kaist_thermal.raw", std::ios::binary);
  if (!f) { std::cerr << "no KAIST\n"; return 1; }
  std::vector<uint8_t> raw(W * H);
  f.read(reinterpret_cast<char*>(raw.data()), raw.size());

  std::vector<uint16_t> src(W * H);
  for (int i = 0; i < W * H; ++i) src[i] = static_cast<uint16_t>(raw[i]) * 64;

  std::vector<int32_t> frame(W * H);
  for (int i = 0; i < W * H; ++i)
    frame[i] = static_cast<int32_t>(src[i]) - 32768;

  f::dwt53_forward_frame_i32(frame, W, H, 1);

  std::vector<int32_t> quant = frame;
  int changed = 0;
  for (auto& c : quant) {
    int32_t q = (c / qstep) * qstep;
    if (q != c) ++changed;
    c = q;
  }
  std::cout << "coeffs changed: " << changed << "\n";

  // Encode via int16 path (Rice still expects int16; clamp for this diagnostic)
  std::vector<int16_t> quant16(W * H);
  for (int i = 0; i < W * H; ++i)
    quant16[i] = static_cast<int16_t>(std::max(-32768, std::min(32767, quant[i])));

  std::vector<uint8_t> bs;
  bool enc_ok = f::encode_subband_blocks(quant16.data(), W, H, 1, 32, bs, nullptr, nullptr);
  std::vector<int16_t> decoded16;
  bool dec_ok = f::decode_subband_blocks(bs.data(), bs.size(), W, H, 1, 32, decoded16);
  std::cout << "enc_ok=" << enc_ok << " dec_ok=" << dec_ok << "\n";

  std::vector<int32_t> recon(W * H);
  for (int i = 0; i < W * H; ++i) recon[i] = decoded16[i];
  f::dwt53_inverse_frame_i32(recon, W, H, 1);

  int32_t rmin = INT32_MAX, rmax = INT32_MIN;
  for (auto v : recon) { rmin = std::min(rmin, v); rmax = std::max(rmax, v); }
  std::cout << "signed inverse-DWT min/max (int32): " << rmin << " / " << rmax << "\n";

  long long max_err = 0;
  int first = -1;
  for (int i = 0; i < W * H; ++i) {
    long long orig = src[i];
    long long rec = static_cast<long long>(recon[i]) + 32768;
    long long err = std::llabs(orig - rec);
    if (err > max_err) max_err = err;
    if (err > 1000 && first < 0) first = i;
  }
  std::cout << "max_abs_err (widened) = " << max_err << "\n";
  if (first >= 0) {
    std::cout << "first large-error pixel at " << first
              << " orig=" << src[first]
              << " recon=" << (recon[first] + 32768) << "\n";
  } else {
    std::cout << "no large-error pixels\n";
  }

  std::cout << "\nV5 frozen. Inverse path widened to int32 for this diagnostic.\n";
  return 0;
}
