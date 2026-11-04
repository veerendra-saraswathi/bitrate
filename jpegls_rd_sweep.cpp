// JPEG-LS reference benchmark (Architecture D)
// Evidence only. CharLS is fetched via CMake FetchContent and is NOT linked
// into bitrate_core or any production binary / MVP v0.2 API.
//
// Primary dataset: data/defense_rd/kaist_*_640x512.raw
// NEAR = 0,1,2,4
// 30 FPS bitrate, median latency after warm-up, PSNR, actual max_err,
// integrity check. Separate CSV per image.
// NEAR=0 must be bit-exact.

#include <charls/charls.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

namespace {

constexpr int kWidth = 640;
constexpr int kHeight = 512;
constexpr int kPixels = kWidth * kHeight;
constexpr double kFps = 30.0;
constexpr int kWarmup = 5;
constexpr int kRepeats = 21;  // odd for clean median

struct Metrics {
  int near{};
  size_t bytes{};
  double bitrate_mbps{};
  double psnr{};
  int max_err{};
  double encode_ms{};
  double decode_ms{};
  bool integrity_ok{};
  bool bit_exact{};
};

std::vector<uint8_t> load_raw(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    throw std::runtime_error("Failed to open " + path);
  }
  std::vector<uint8_t> data(kPixels);
  f.read(reinterpret_cast<char*>(data.data()), kPixels);
  if (f.gcount() != kPixels) {
    throw std::runtime_error("Short read on " + path + " (expected " +
                             std::to_string(kPixels) + " bytes)");
  }
  return data;
}

double median(std::vector<double>& v) {
  if (v.empty()) return 0.0;
  std::sort(v.begin(), v.end());
  return v[v.size() / 2];
}

double compute_psnr(const std::vector<uint8_t>& orig,
                    const std::vector<uint8_t>& recon) {
  double mse = 0.0;
  for (int i = 0; i < kPixels; ++i) {
    const double d = static_cast<double>(orig[i]) - static_cast<double>(recon[i]);
    mse += d * d;
  }
  mse /= kPixels;
  if (mse == 0.0) return std::numeric_limits<double>::infinity();
  return 10.0 * std::log10((255.0 * 255.0) / mse);
}

int compute_max_err(const std::vector<uint8_t>& orig,
                    const std::vector<uint8_t>& recon) {
  int max_e = 0;
  for (int i = 0; i < kPixels; ++i) {
    const int d = std::abs(static_cast<int>(orig[i]) - static_cast<int>(recon[i]));
    if (d > max_e) max_e = d;
  }
  return max_e;
}

bool pixels_equal(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
  return a == b;
}

Metrics run_near(const std::vector<uint8_t>& original, int near) {
  using namespace charls;
  using clock = std::chrono::steady_clock;

  jpegls_encoder encoder;
  encoder.frame_info({kWidth, kHeight, 8, 1})
         .interleave_mode(interleave_mode::none)
         .near_lossless(near);

  const size_t dest_size = encoder.estimated_destination_size();
  std::vector<uint8_t> encoded(dest_size);

  // Warm-up
  for (int i = 0; i < kWarmup; ++i) {
    encoder.destination(encoded);
    encoder.encode(original);
  }

  // Encode timing
  std::vector<double> encode_times;
  encode_times.reserve(kRepeats);
  size_t final_bytes = 0;
  for (int i = 0; i < kRepeats; ++i) {
    auto t0 = clock::now();
    encoder.destination(encoded);
    const size_t written = encoder.encode(original);
    auto t1 = clock::now();
    encode_times.push_back(
        std::chrono::duration<double, std::milli>(t1 - t0).count());
    final_bytes = written;
  }
  encoded.resize(final_bytes);

  // Decode timing + verification
  std::vector<double> decode_times;
  decode_times.reserve(kRepeats);
  std::vector<uint8_t> decoded(kPixels);
  bool integrity = true;
  int max_err = 0;
  double psnr = 0.0;

  for (int i = 0; i < kRepeats; ++i) {
    jpegls_decoder decoder{encoded, true};
    auto t0 = clock::now();
    decoder.decode(decoded);
    auto t1 = clock::now();
    decode_times.push_back(
        std::chrono::duration<double, std::milli>(t1 - t0).count());

    // Verify on last iteration (or every, but last is enough for report)
    if (i == kRepeats - 1) {
      max_err = compute_max_err(original, decoded);
      psnr = compute_psnr(original, decoded);
      integrity = (decoder.frame_info().width == kWidth &&
                   decoder.frame_info().height == kHeight &&
                   decoder.frame_info().bits_per_sample == 8 &&
                   decoder.frame_info().component_count == 1);
      if (near == 0) {
        integrity = integrity && pixels_equal(original, decoded);
      }
    }
  }

  Metrics m;
  m.near = near;
  m.bytes = final_bytes;
  m.bitrate_mbps = (static_cast<double>(final_bytes) * 8.0 * kFps) / 1e6;
  m.psnr = psnr;
  m.max_err = max_err;
  m.encode_ms = median(encode_times);
  m.decode_ms = median(decode_times);
  m.integrity_ok = integrity;
  m.bit_exact = (near == 0) && pixels_equal(original, decoded);
  return m;
}

void write_csv(const std::string& path, const std::vector<Metrics>& results) {
  std::ofstream out(path);
  if (!out) {
    throw std::runtime_error("Failed to write " + path);
  }
  // Preserve existing RD sweep CSV conventions (clear, parseable headers)
  out << "near,bytes,bitrate_mbps_30fps,psnr_db,max_err,encode_ms_median,decode_ms_median,integrity_ok,bit_exact\n";
  for (const auto& m : results) {
    out << m.near << ","
        << m.bytes << ","
        << m.bitrate_mbps << ","
        << (std::isinf(m.psnr) ? "inf" : std::to_string(m.psnr)) << ","
        << m.max_err << ","
        << m.encode_ms << ","
        << m.decode_ms << ","
        << (m.integrity_ok ? 1 : 0) << ","
        << (m.bit_exact ? 1 : 0) << "\n";
  }
  std::cout << "Wrote " << path << "\n";
}

void bench_one(const std::string& raw_path, const std::string& csv_path,
               const std::string& label) {
  std::cout << "\n=== " << label << " ===\n";
  const auto original = load_raw(raw_path);
  std::cout << "Loaded " << raw_path << " (" << kPixels << " bytes)\n";

  const int nears[] = {0, 1, 2, 4};
  std::vector<Metrics> results;
  results.reserve(4);

  for (int near : nears) {
    std::cout << "NEAR=" << near << " ... ";
    std::cout.flush();
    auto m = run_near(original, near);
    results.push_back(m);
    std::cout << m.bitrate_mbps << " Mbps, PSNR="
              << (std::isinf(m.psnr) ? "inf" : std::to_string(m.psnr))
              << " dB, max_err=" << m.max_err
              << ", enc=" << m.encode_ms << " ms, dec=" << m.decode_ms << " ms"
              << ", integrity=" << (m.integrity_ok ? "OK" : "FAIL")
              << (m.bit_exact ? " (bit-exact)" : "") << "\n";

    if (near == 0 && !m.bit_exact) {
      std::cerr << "ERROR: NEAR=0 did not achieve bit-exact reconstruction!\n";
    }
    if (!m.integrity_ok) {
      std::cerr << "ERROR: integrity check failed for NEAR=" << near << "\n";
    }
  }

  write_csv(csv_path, results);
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const std::string base = (argc > 1) ? argv[1] : "data/defense_rd";
    const std::string out_dir = (argc > 2) ? argv[2] : "results";

    bench_one(base + "/kaist_thermal_640x512.raw",
              out_dir + "/jpegls_rd_sweep_thermal_640x512_30fps.csv",
              "KAIST thermal 640x512");

    bench_one(base + "/kaist_visible_640x512.raw",
              out_dir + "/jpegls_rd_sweep_visible_640x512_30fps.csv",
              "KAIST visible 640x512");

    std::cout << "\nJPEG-LS reference benchmark complete.\n";
    std::cout << "NEAR=0 must be bit-exact. Actual max_err reported for NEAR>0.\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}
