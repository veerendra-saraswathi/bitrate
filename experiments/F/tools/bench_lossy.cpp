#include "../include/f_lossy.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

static std::vector<uint16_t> load_u16(const std::string& path, int w, int h) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::vector<uint16_t> data(w * h);
  f.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint16_t));
  return data;
}

static std::vector<uint16_t> load_u8_as_u16(const std::string& path, int w, int h) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::vector<uint8_t> raw(w * h);
  f.read(reinterpret_cast<char*>(raw.data()), raw.size());
  std::vector<uint16_t> data(w * h);
  for (int i = 0; i < w * h; ++i) data[i] = raw[i] * 64; // stretch 8-bit into 14-bit range
  return data;
}

static void run_case(const char* name, const std::vector<uint16_t>& src, int w, int h) {
  if (src.empty()) {
    std::cout << "\n=== " << name << " === SKIPPED (file not found)\n";
    return;
  }
  const size_t BUDGET = 8333;
  std::cout << "\n=== " << name << " ===\n";
  std::cout << "qstep  bytes  budget_met  PSNR  RMSE  max_err  enc_ms\n";
  for (int q : {1, 4, 16, 64, 256, 1024, 4096}) {
    f::LossyResult r;
    f::encode_lossy_frame_u16(src.data(), w, h, q, BUDGET, r);
    std::cout << q << "  " << r.bytes << "  " << (r.budget_met ? "yes" : "no")
              << "  " << r.psnr << "  " << r.rmse << "  " << r.max_err
              << "  " << r.encode_ms << "\n";
    if (r.budget_met) break;
  }
}

int main(int argc, char** argv) {
  const int W = 640, H = 512;

  // Real native 14-bit IR (if provided)
  std::string ir_path = (argc > 1) ? argv[1] : "";
  if (!ir_path.empty()) {
    auto src = load_u16(ir_path, W, H);
    run_case("native 14-bit IR", src, W, H);
  } else {
    std::cout << "No native 14-bit IR path given. Pass one as argv[1] if available.\n";
  }

  // KAIST supplementary (8-bit stretched into 14-bit range)
  auto kaist = load_u8_as_u16("../../../data/kaist_thermal.raw", W, H);
  run_case("KAIST thermal (8-bit stretched)", kaist, W, H);

  std::cout << "\nRecord: budget missed; quality measurements unverified on real 14-bit IR (pending file).\n";
  std::cout << "Rice-only. rANS disabled. Lossless path unchanged.\n";
  return 0;
}
