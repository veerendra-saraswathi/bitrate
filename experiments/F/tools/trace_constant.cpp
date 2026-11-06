#include "../include/f_lossy.h"
#include "../include/f_codec.h"
#include "../src/wavelet53.h"
#include "../src/blocks.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

static void dump_some(const char* label, const std::vector<int16_t>& v, int n = 8) {
  std::cout << label << ": ";
  for (int i = 0; i < n && i < (int)v.size(); ++i) std::cout << v[i] << " ";
  std::cout << "\n";
}

static void trace_constant(int qstep) {
  const int W = 64, H = 64;
  std::vector<uint16_t> src(W * H, 8000);
  std::cout << "\n===== TRACE qstep=" << qstep << " (constant 8000, " << W << "x" << H << ") =====\n";

  std::vector<int16_t> frame(W * H);
  for (int i = 0; i < W * H; ++i) frame[i] = static_cast<int16_t>(src[i] - 32768);
  dump_some("input (centered)", frame);

  f::dwt53_forward_frame(frame, W, H, 1);
  dump_some("after forward DWT", frame);

  std::vector<int16_t> quant = frame;
  if (qstep < 1) qstep = 1;
  int changed = 0;
  for (auto& c : quant) {
    int16_t q = static_cast<int16_t>((c / qstep) * qstep);
    if (q != c) ++changed;
    c = q;
  }
  std::cout << "coeffs changed by quant: " << changed << "\n";
  dump_some("after quant", quant);

  std::vector<uint8_t> bs;
  bool enc_ok = f::encode_subband_blocks(quant.data(), W, H, 1, 32, bs, nullptr, nullptr);
  std::cout << "encode_ok=" << enc_ok << " payload_bytes=" << bs.size() << "\n";

  std::vector<int16_t> decoded;
  bool dec_ok = f::decode_subband_blocks(bs.data(), bs.size(), W, H, 1, 32, decoded);
  std::cout << "decode_ok=" << dec_ok << " decoded_samples=" << decoded.size() << "\n";
  dump_some("after decode (coeffs)", decoded);

  std::vector<int16_t> recon = decoded;
  f::dwt53_inverse_frame(recon, W, H, 1);
  dump_some("after inverse DWT (raw)", recon);

  long long max_abs = 0;
  double mse = 0;
  for (int i = 0; i < W * H; ++i) {
    long long orig = static_cast<long long>(src[i]);
    long long rec = static_cast<long long>(recon[i]) + 32768;
    long long d = orig - rec;
    max_abs = std::max(max_abs, std::llabs(d));
    mse += static_cast<double>(d) * d;
  }
  mse /= (W * H);
  std::cout << "raw max_abs_err (no clamp) = " << max_abs << "\n";
  std::cout << "raw RMSE = " << std::sqrt(mse) << "\n";
}

int main() {
  trace_constant(1);
  trace_constant(256);
  std::cout << "\nV5 lossless path unchanged. rANS disabled.\n";
  return 0;
}
