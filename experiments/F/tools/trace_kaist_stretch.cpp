#include "../include/f_codec.h"
#include "../src/wavelet53.h"
#include "../src/blocks.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

static std::vector<uint8_t> load_crop(const std::string& path, int full_w, int full_h,
                                      int x0, int y0, int cw, int ch) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::vector<uint8_t> full(full_w * full_h);
  f.read(reinterpret_cast<char*>(full.data()), full.size());
  std::vector<uint8_t> crop(cw * ch);
  for (int y = 0; y < ch; ++y)
    for (int x = 0; x < cw; ++x)
      crop[y * cw + x] = full[(y0 + y) * full_w + (x0 + x)];
  return crop;
}

static void trace_crop_stretched(const std::vector<uint8_t>& src8, int W, int H, int qstep) {
  std::cout << "\n===== TRACE KAIST crop STRETCHED qstep=" << qstep << " =====\n";

  // Stretch 8-bit into 14-bit range the same way the earlier bench did
  std::vector<uint16_t> src(W * H);
  for (int i = 0; i < W * H; ++i) src[i] = static_cast<uint16_t>(src8[i]) * 64;

  std::vector<int16_t> frame(W * H);
  for (int i = 0; i < W * H; ++i)
    frame[i] = static_cast<int16_t>(static_cast<int>(src[i]) - 32768); // same centering as lossy path

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

  bool coeff_match = quant.size() == decoded.size();
  if (coeff_match)
    for (size_t i = 0; i < quant.size(); ++i)
      if (quant[i] != decoded[i]) { coeff_match = false; break; }
  std::cout << "quantised == decoded: " << (coeff_match ? "yes" : "NO") << "\n";

  std::vector<int16_t> recon_centered = decoded;
  f::dwt53_inverse_frame(recon_centered, W, H, 1);

  int16_t rmin = 32767, rmax = -32768;
  for (auto v : recon_centered) { rmin = std::min(rmin, v); rmax = std::max(rmax, v); }
  std::cout << "recon signed min/max (centered, before +32768): " << rmin << " / " << rmax << "\n";

  long long max_abs = 0;
  int first = -1;
  for (int i = 0; i < W * H; ++i) {
    long long orig = src[i];
    long long rec = static_cast<long long>(recon_centered[i]) + 32768;
    long long d = orig - rec;
    if (d != 0 && first < 0) first = i;
    max_abs = std::max(max_abs, std::llabs(d));
  }
  std::cout << "raw max_abs_err (widened, after +32768) = " << max_abs << "\n";
  if (first >= 0)
    std::cout << "first mismatch at " << first
              << " orig=" << src[first]
              << " recon=" << (static_cast<long long>(recon_centered[first]) + 32768) << "\n";
  else
    std::cout << "no pixel mismatch\n";
}

int main() {
  const int FW = 640, FH = 512, CW = 64, CH = 64;
  auto crop = load_crop("../../../data/kaist_thermal.raw", FW, FH, 100, 100, CW, CH);
  if (crop.empty()) { std::cerr << "Cannot load crop\n"; return 1; }
  trace_crop_stretched(crop, CW, CH, 1);
  trace_crop_stretched(crop, CW, CH, 256);
  std::cout << "\nV5 frozen. rANS disabled.\n";
  return 0;
}
