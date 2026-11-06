#include "../include/f_lossy.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

static std::vector<uint16_t> load_u8_as_u16(const std::string& path, int w, int h) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::vector<uint8_t> raw(w * h);
  f.read(reinterpret_cast<char*>(raw.data()), raw.size());
  std::vector<uint16_t> data(w * h);
  for (int i = 0; i < w * h; ++i)
    data[i] = static_cast<uint16_t>(raw[i]) * 64; // stretch
  return data;
}

static void run(const char* name, const std::vector<uint16_t>& src, int w, int h) {
  if (src.empty()) { std::cout << name << " SKIPPED\n"; return; }
  const size_t BUDGET = 8333;
  std::cout << "\n=== " << name << " ===\n";
  std::cout << "qstep  bytes  budget_met  PSNR  RMSE  max_err  enc_ms\n";
  for (int q : {1, 4, 16, 64, 256, 1024, 4096}) {
    f::LossyResult r;
    f::encode_lossy_frame_u16(src.data(), w, h, q, BUDGET, r);
    std::cout << q << "  " << r.bytes << "  " << (r.budget_met ? "yes" : "no")
              << "  " << r.psnr << "  " << r.rmse << "  " << r.max_err
              << "  " << r.encode_ms << "\n";
  }
}

int main() {
  const int W = 640, H = 512;

  // KAIST thermal stretched (same as earlier failing case)
  auto kaist = load_u8_as_u16("../../../data/kaist_thermal.raw", W, H);
  run("KAIST thermal stretched (corrected metrics)", kaist, W, H);

  // Synthetic 14-bit for comparison
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> dist(0, 16383);
  std::vector<uint16_t> syn(W * H);
  for (auto& v : syn) v = static_cast<uint16_t>(dist(rng));
  run("synthetic 14-bit", syn, W, H);

  std::cout << "\nV5 frozen. rANS disabled. Metrics use widened signed arithmetic.\n";
  return 0;
}
