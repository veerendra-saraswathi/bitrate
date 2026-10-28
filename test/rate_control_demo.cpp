#include "rate_control.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: rate_control_demo <thermal|visible> <target_bpp>\n"
                  << "Example: rate_control_demo visible 4.0\n";
        return 1;
    }

    std::string curve = argv[1];
    double target = std::stod(argv[2]);

    int qp = select_qp_for_target_bpp(curve, target);
    if (qp < 0) {
        std::cerr << "Failed to select QP\n";
        return 1;
    }

    std::cout << "curve=" << curve
              << " target_bpp=" << target
              << " -> selected QP=" << qp << "\n";

    // Also show a few neighbouring points for context
    auto pts = (curve == "visible" || curve == "vis")
               ? load_kaist_visible_levels1()
               : load_kaist_thermal_levels1();
    std::cout << "curve points (QP,bpp):\n";
    for (const auto& p : pts) {
        std::cout << "  QP=" << p.qp << " bpp=" << p.bpp;
        if (p.qp == qp) std::cout << "  <-- selected";
        std::cout << "\n";
    }
    return 0;
}
