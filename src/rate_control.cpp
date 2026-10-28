#include "rate_control.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::vector<QpPoint> load_qp_curve(const std::string& csv_path) {
    std::vector<QpPoint> curve;
    std::ifstream f(csv_path);
    if (!f) return curve;

    std::string line;
    // Skip header
    if (!std::getline(f, line)) return curve;

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        QpPoint p;
        int col = 0;
        while (std::getline(ss, cell, ',')) {
            try {
                switch (col) {
                    case 0: p.qp = std::stoi(cell); break;
                    case 1: p.bytes = static_cast<uint32_t>(std::stoul(cell)); break;
                    case 2: p.bpp = std::stod(cell); break;
                    case 3: p.enc_ms = std::stod(cell); break;
                    case 4: p.psnr = std::stod(cell); break;
                    case 5: p.ssim = std::stod(cell); break;
                }
            } catch (...) {
                // malformed row — skip
                p.qp = -1;
                break;
            }
            ++col;
        }
        if (p.qp >= 0 && col >= 3)
            curve.push_back(p);
    }

    // Ensure ascending QP order for predictable selection
    std::sort(curve.begin(), curve.end(),
              [](const QpPoint& a, const QpPoint& b) { return a.qp < b.qp; });
    return curve;
}

int select_qp_for_target_bpp(const std::vector<QpPoint>& curve, double target_bpp) {
    if (curve.empty()) return -1;

    // Highest quality (lowest QP) point
    if (target_bpp >= curve.front().bpp)
        return curve.front().qp;

    // Walk from coarsest (highest QP) toward finer until we find a point that still meets the target
    for (auto it = curve.rbegin(); it != curve.rend(); ++it) {
        if (it->bpp <= target_bpp)
            return it->qp;
    }
    // Target is stricter than every measured point → return coarsest
    return curve.back().qp;
}

std::vector<QpPoint> load_kaist_thermal_levels1() {
    // Prefer the levels=1 file; fall back to the older full-curve file if present
    auto c = load_qp_curve("results/kaist_thermal_levels1_qp_curve.csv");
    if (c.empty())
        c = load_qp_curve("results/kaist_thermal_qp_curve.csv");
    return c;
}

std::vector<QpPoint> load_kaist_visible_levels1() {
    return load_qp_curve("results/kaist_visible_levels1_qp_curve.csv");
}

int select_qp_for_target_bpp(const std::string& curve_name, double target_bpp) {
    std::string name = to_lower(curve_name);
    std::vector<QpPoint> curve;
    if (name == "visible" || name == "vis" || name == "rgb")
        curve = load_kaist_visible_levels1();
    else
        curve = load_kaist_thermal_levels1();   // default

    if (curve.empty()) {
        std::cerr << "rate_control: could not load curve '" << curve_name << "'\n";
        return -1;
    }
    return select_qp_for_target_bpp(curve, target_bpp);
}
