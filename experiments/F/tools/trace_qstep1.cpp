#include "../include/f_lossy.h"
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
  const int qstep = 1;

  std::ifstream f("../../../data/kaist_thermal.raw", std::ios::binary);
  if (!f) { std::cerr << "no KAIST\n"; return 1; }
  std::vector<uint8_t> raw(W * H);
  f.read(reinterpret_cast<char*>(raw.data()), raw.size());

  std::vector<uint16_t> src(W * H);
  for (int i = 0; i < W * H; ++i) src[i] = static_cast<uint16_t>(raw[i]) * 64;

  std::cout << "qstep=1 exactness trace (KAIST thermal stretched)\n";
  std::cout << "Declared range: 0 .. 16383\n";

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
  std::cout << "coeffs changed by quant: " << changed << "\n";

  // Narrow to int16 for Rice
  std::vector<int16_t> quant16(W * H);
  int narrowed = 0;
  for (int i = 0; i < W * H; ++i) {
    int32_t v = quant[i];
    if (v < -32768 || v > 32767) ++narrowed;
    quant16[i] = static_cast<int16_t>(std::max(-32768, std::min(32767, v)));
  }
  std::cout << "values narrowed to int16: " << narrowed << "\n";

  std::vector<uint8_t> bs;
  bool enc_ok = f::encode_subband_blocks(quant16.data(), W, H, 1, 32, bs, nullptr, nullptr);
  std::vector<int16_t> decoded16;
  bool dec_ok = f::decode_subband_blocks(bs.data(), bs.size(), W, H, 1, 32, decoded16);
  std::cout << "enc_ok=" << enc_ok << " dec_ok=" << dec_ok << "\n";

  int coeff_mm = 0;
  int first_coeff_mm = -1;
  for (size_t i = 0; i < quant16.size(); ++i) {
    if (quant16[i] != decoded16[i]) {
      if (first_coeff_mm < 0) first_coeff_mm = static_cast<int>(i);
      ++coeff_mm;
    }
  }
  std::cout << "quant16 vs decoded16 mismatches: " << coeff_mm;
  if (first_coeff_mm >= 0) std::cout << " (first at " << first_coeff_mm << ")";
  std::cout << "\n";

  std::vector<int32_t> recon(W * H);
  for (int i = 0; i < W * H; ++i) recon[i] = decoded16[i];
  f::dwt53_inverse_frame_i32(recon, W, H, 1);

  int32_t rmin = INT32_MAX, rmax = INT32_MIN;
  for (auto v : recon) { rmin = std::min(rmin, v); rmax = std::max(rmax, v); }
  std::cout << "signed inverse min/max: " << rmin << " / " << rmax << "\n";

  long long max_err = 0;
  int first = -1;
  int clipped = 0;
  for (int i = 0; i < W * H; ++i) {
    long long orig = src[i];
    long long raw_rec = static_cast<long long>(recon[i]) + 32768;
    long long final_rec = raw_rec;
    if (final_rec < 0 || final_rec > 65535) ++clipped;
    if (final_rec < 0) final_rec = 0;
    if (final_rec > 65535) final_rec = 65535;
    long long err = std::llabs(orig - final_rec);
    if (err > max_err) max_err = err;
    if (err != 0 && first < 0) first = i;
  }
  std::cout << "max_abs_err = " << max_err << "\n";
  std::cout << "pixels that required clipping: " << clipped << "\n";
  if (first >= 0) {
    std::cout << "first mismatch at " << first << "\n";
    std::cout << "  original     = " << src[first] << "\n";
    std::cout << "  signed recon = " << recon[first] << "\n";
    std::cout << "  after +32768 = " << (recon[first] + 32768) << "\n";
    std::cout << "  final uint16 = " << std::max(0, std::min(65535, recon[first] + 32768)) << "\n";
  } else {
    std::cout << "perfect match\n";
  }

  std::cout << "\nV5 frozen. qstep=1 exactness diagnostic.\n";
  return 0;
}
