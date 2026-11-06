#include "../include/f_codec.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>

static std::vector<uint8_t> load_raw(const std::string& path, int w, int h) {
  std::ifstream f(path, std::ios::binary);
  if (!f) { std::cerr << "Cannot open " << path << "\n"; return {}; }
  std::vector<uint8_t> data(w * h);
  f.read(reinterpret_cast<char*>(data.data()), data.size());
  return data;
}

static int max_abs_err(const uint8_t* a, const uint8_t* b, int n) {
  int m = 0;
  for (int i = 0; i < n; ++i) m = std::max(m, std::abs(int(a[i]) - int(b[i])));
  return m;
}

static double median(std::vector<double>& v) {
  if (v.empty()) return 0;
  std::sort(v.begin(), v.end());
  return v[v.size()/2];
}

static void print_subbands(const std::vector<f::SubbandStat>& sbs) {
  std::cout << "  subband  lvl type   wxh     cmin  cmax  zero%   avg_k  payload_B  blocks\n";
  for (const auto& s : sbs) {
    std::cout << "  " << s.x << "," << s.y << "  " << s.level << " " << s.type
              << "  " << s.w << "x" << s.h
              << "  " << s.cmin << "  " << s.cmax
              << "  " << (s.zero_frac*100) << "  " << s.avg_k
              << "  " << s.payload_bytes << "  " << s.blocks << "\n";
  }
}

static void test_14bit_and_odd() {
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> dist(0, 16383); // 14-bit
  std::vector<std::pair<int,int>> sizes = {{640,512},{641,513},{320,240},{17,13}};
  std::cout << "\n=== 14-bit / odd-size round-trip ===\n";
  for (auto [w,h] : sizes) {
    std::vector<uint16_t> src(w*h);
    for (auto& v : src) v = static_cast<uint16_t>(dist(rng));
    std::vector<uint8_t> bs;
    bool eok = f::encode_frame_u16(src.data(), w, h, 1, 32, bs);
    std::vector<uint16_t> rec;
    bool dok = f::decode_frame_u16(bs.data(), bs.size(), w, h, 1, 32, rec);
    bool exact = dok && (src == rec);
    std::cout << w << "x" << h << "  bytes=" << bs.size()
              << "  exact=" << (exact ? "yes" : "no")
              << "  eok=" << eok << " dok=" << dok << "\n";
  }
}

int main(int argc, char** argv) {
  const int W = 640, H = 512;
  const int block_size = 32;
  const int levels = 1; // frozen baseline
  std::string thermal = (argc > 1) ? argv[1] : "../../../data/kaist_thermal.raw";
  std::string visible = (argc > 2) ? argv[2] : "../../../data/kaist_visible.raw";

  auto run = [&](const std::string& name, const std::string& path) {
    auto src = load_raw(path, W, H);
    if (src.empty()) return;
    std::cout << "\n=== " << name << " (level " << levels << ") ===\n";

    std::vector<uint8_t> bs;
    f::EncodeStats st;
    std::vector<f::SubbandStat> sbs;
    f::encode_frame(src.data(), W, H, levels, block_size, bs, &st, &sbs);

    std::vector<double> times;
    for (int r = 0; r < 11; ++r) {
      auto t0 = std::chrono::high_resolution_clock::now();
      f::encode_frame(src.data(), W, H, levels, block_size, bs, &st, nullptr);
      times.push_back(std::chrono::duration<double,std::milli>(
          std::chrono::high_resolution_clock::now() - t0).count());
    }
    double med = median(times);

    std::vector<uint8_t> rec;
    bool dok = f::decode_frame(bs.data(), bs.size(), W, H, levels, block_size, rec);
    int maxe = max_abs_err(src.data(), rec.data(), W*H);
    bool exact = dok && maxe == 0;
    double mbps = (st.total_bytes * 8.0 * 30.0) / 1e6;

    std::cout << "payload_B=" << st.payload_bytes
              << " header_B=" << st.header_bytes
              << " total_B=" << st.total_bytes
              << " Mbps@30=" << mbps
              << " bit_exact=" << (exact?"yes":"no")
              << " max_err=" << maxe
              << " enc_ms_med=" << med << "\n";
    std::cout << "timing: transform=" << st.transform_ms
              << " ms  analysis+payload=" << (st.analysis_ms + st.payload_ms)
              << " ms  total=" << st.total_ms << " ms\n";
    print_subbands(sbs);
  };

  run("KAIST thermal", thermal);
  run("KAIST visible", visible);
  test_14bit_and_odd();
  return 0;
}
