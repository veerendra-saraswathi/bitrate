#include "../include/f_codec.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>
#include <numeric>

static double median(std::vector<double>& v) {
  if (v.empty()) return 0;
  std::sort(v.begin(), v.end());
  return v[v.size() / 2];
}

static void measure_14bit_lossless() {
  std::cout << "\n=== Native 14-bit IR lossless baseline (V5 path, level 1) ===\n";
  std::mt19937 rng(20261106);
  std::uniform_int_distribution<int> dist(0, 16383);

  struct Case { int w, h; const char* name; };
  Case cases[] = {{640,512,"640x512"}, {641,513,"641x513"}, {320,240,"320x240"}};

  for (auto c : cases) {
    std::vector<uint16_t> src(c.w * c.h);
    for (auto& v : src) v = static_cast<uint16_t>(dist(rng));

    std::vector<uint8_t> bs;
    std::vector<double> times;
    for (int r = 0; r < 7; ++r) {
      auto t0 = std::chrono::high_resolution_clock::now();
      f::encode_frame_u16(src.data(), c.w, c.h, 1, 32, bs);
      times.push_back(std::chrono::duration<double,std::milli>(
          std::chrono::high_resolution_clock::now() - t0).count());
    }
    double med = median(times);

    std::vector<uint16_t> rec;
    auto t0 = std::chrono::high_resolution_clock::now();
    bool dok = f::decode_frame_u16(bs.data(), bs.size(), c.w, c.h, 1, 32, rec);
    double dec_ms = std::chrono::duration<double,std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    int maxe = 0;
    if (dok) {
      for (size_t i = 0; i < src.size(); ++i)
        maxe = std::max(maxe, std::abs(int(src[i]) - int(rec[i])));
    }
    bool exact = dok && maxe == 0;
    double mbps = (bs.size() * 8.0 * 30.0) / 1e6;

    std::cout << c.name
              << "  bytes=" << bs.size()
              << "  Mbps@30=" << mbps
              << "  enc_ms_med=" << med
              << "  dec_ms=" << dec_ms
              << "  max_err=" << maxe
              << "  bit_exact=" << (exact ? "yes" : "no") << "\n";
  }
}

int main() {
  measure_14bit_lossless();
  std::cout << "\nV5 lossless path unchanged. rANS disabled.\n";
  return 0;
}
