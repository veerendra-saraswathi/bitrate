#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

struct QpPoint {
    int qp = 0;
    uint32_t bytes = 0;
    double bpp = 0.0;
    double enc_ms = 0.0;
    double psnr = 0.0;
    double ssim = 0.0;
};

// Load a measured QP curve from CSV produced by bench_quality.
// Expected header: QP,bytes,bpp,enc_ms,psnr_dB,ssim
// Returns empty vector on failure.
std::vector<QpPoint> load_qp_curve(const std::string& csv_path);

// From a loaded curve, return the highest QP whose measured bpp is still <= target_bpp.
// If target_bpp is higher than the lowest-QP (best quality) point, returns that QP (usually 1).
// If target_bpp is lower than every point, returns the highest QP in the curve.
// Returns -1 if the curve is empty.
int select_qp_for_target_bpp(const std::vector<QpPoint>& curve, double target_bpp);

// Convenience: load the known KAIST levels=1 curves that ship in results/.
// Returns empty if the file is missing or unreadable.
std::vector<QpPoint> load_kaist_thermal_levels1();
std::vector<QpPoint> load_kaist_visible_levels1();

// High-level helper used by tools: try to satisfy target_bpp using a named curve.
// curve_name: "thermal" or "visible" (case-insensitive). Falls back to thermal.
// Returns the selected QP, or -1 on failure.
int select_qp_for_target_bpp(const std::string& curve_name, double target_bpp);
