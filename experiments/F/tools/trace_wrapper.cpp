#include "../include/f_lossy.h"
#include "../include/f_codec.h"
#include "../src/wavelet53.h"
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

  std::cout << "Declared bit depth: 14 (stretched 8-bit)\n";
  std::cout << "Reconstruction expected range: 0 .. 16383\n";

  std::vector<int16_t> frame(W * H);
  for (int i = 0; i < W * H; ++i)
    frame[i] = static_cast<int16_t>(static_cast<int>(src[i]) - 32768);

  f::dwt53_forward_frame(frame, W, H, 1);

  std::vector<int16_t> quant = frame;
  int changed = 0;
  for (auto& c : quant) {
    int16_t q = static_cast<int16_t>((c / qstep) * qstep);
    if (q != c) ++changed;
    c = q;
  }
  std::cout << "coeffs changed: " << changed << "\n";

  std::vector<uint8_t> bs;
  bool enc_ok = f::encode_subband_blocks(quant.data(), W, H, 1, 32, bs, nullptr, nullptr);
  std::vector<int16_t> decoded;
  bool dec_ok = f::decode_subband_blocks(bs.data(), bs.size(), W, H, 1, 32, decoded);
  std::cout << "enc_ok=" << enc_ok << " dec_ok=" << dec_ok << "\n";

  // Check ALL quantised vs decoded
  int mismatches = 0;
  int first_mm = -1;
  for (size_t i = 0; i < quant.size(); ++i) {
    if (quant[i] != decoded[i]) {
      if (first_mm < 0) first_mm = static_cast<int>(i);
      ++mismatches;
    }
  }
  std::cout << "quantised vs decoded mismatches: " << mismatches;
  if (first_mm >= 0) std::cout << " (first at " << first_mm << ")";
  std::cout << "\n";

  std::vector<int16_t> recon_centered = decoded;
  f::dwt53_inverse_frame(recon_centered, W, H, 1);

  int16_t rmin = 32767, rmax = -32768;
  for (auto v : recon_centered) { rmin = std::min(rmin, v); rmax = std::max(rmax, v); }
  std::cout << "signed inverse-DWT min/max: " << (int)rmin << " / " << (int)rmax << "\n";

  // Find first large-error pixel
  for (int i = 0; i < W * H; ++i) {
    long long orig = src[i];
    long long signed_recon = recon_centered[i];
    long long after_offset = signed_recon + 32768;
    uint16_t final_u16 = static_cast<uint16_t>(std::max(0LL, std::min(65535LL, after_offset)));
    long long err = std::llabs(orig - static_cast<long long>(final_u16));
    if (err > 1000) {
      std::cout << "\nFirst large-error pixel at " << i << "\n";
      std::cout << "  original          = " << orig << "\n";
      std::cout << "  signed inverse-DWT= " << signed_recon << "\n";
      std::cout << "  after +32768      = " << after_offset << "\n";
      std::cout << "  final uint16_t    = " << final_u16 << "\n";
      std::cout << "  abs error         = " << err << "\n";
      break;
    }
  }

  std::cout << "\nV5 frozen. No clamping added.\n";
  return 0;
}
