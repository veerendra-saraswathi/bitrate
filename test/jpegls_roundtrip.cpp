#include "jpegls.h"
#include <cstdio>
#include <fstream>
#include <vector>

int main() {
    const int w = 640, h = 512;
    std::ifstream f("data/defense_rd/kaist_thermal_640x512.raw", std::ios::binary);
    if (!f) {
        std::perror("open raw");
        return 1;
    }
    std::vector<uint8_t> raw(w * h);
    f.read(reinterpret_cast<char*>(raw.data()), raw.size());

    jpegls_encoder* enc = jpegls_encoder_create();
    jpegls_encoder_set_frame_info(enc, w, h, 8, 1);
    jpegls_encoder_set_near(enc, 0);

    std::vector<uint8_t> out(w * h * 2); // generous buffer
    int n = jpegls_encoder_encode(enc, raw.data(), raw.size(), out.data(), out.size());
    std::printf("Wrote %d bytes\n", n);

    if (n > 0) {
        std::ofstream ofs("thermal.jls", std::ios::binary);
        ofs.write(reinterpret_cast<char*>(out.data()), n);
        std::printf("Also wrote thermal.jls\n");
    }

    jpegls_encoder_destroy(enc);
    return n > 0 ? 0 : 1;
}
