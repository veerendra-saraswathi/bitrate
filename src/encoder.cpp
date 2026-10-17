#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <thread>
#include <mutex>

static void write_val(std::vector<uint8_t>& out, int32_t val) {
    uint32_t u = (val >= 0) ? (static_cast<uint32_t>(val) << 1)
                            : (static_cast<uint32_t>(-val) << 1) - 1;
    if (u < 64) {
        out.push_back(static_cast<uint8_t>(u));
    } else {
        out.push_back(0x40 | static_cast<uint8_t>((u >> 8) & 0x3F));
        out.push_back(static_cast<uint8_t>(u & 0xFF));
    }
}

static void rice_encode(std::vector<uint8_t>& out, const std::vector<int32_t>& coeffs) {
    int run = 0;
    for (int32_t c : coeffs) {
        if (c == 0) {
            ++run;
            if (run == 255) { out.push_back(0xFF); run = 0; }
        } else {
            if (run) { out.push_back(static_cast<uint8_t>(run)); run = 0; }
            else     { out.push_back(0); }
            write_val(out, c);
        }
    }
    if (run) out.push_back(static_cast<uint8_t>(run));
}

struct TileJob {
    int ty, tx, tw, th;
    std::vector<uint8_t> payload;
};

static void encode_one_tile(TileJob& job,
                            const std::vector<uint16_t>& plane,
                            int width, int qp) {
    std::vector<int32_t> t(job.tw * job.th);
    for (int y = 0; y < job.th; ++y)
        for (int x = 0; x < job.tw; ++x)
            t[y * job.tw + x] = plane[(job.ty + y) * width + (job.tx + x)];
    wavelet_5_3_forward(t, job.tw, job.th);
    quantise(t, qp);
    rice_encode(job.payload, t);
}

static void encode_plane_parallel(std::vector<uint8_t>& payload,
                                  const std::vector<uint16_t>& plane,
                                  int width, int height, int qp) {
    const int tile = 64;
    std::vector<TileJob> jobs;
    for (int ty = 0; ty < height; ty += tile)
        for (int tx = 0; tx < width; tx += tile) {
            TileJob j;
            j.ty = ty; j.tx = tx;
            j.tw = std::min(tile, width  - tx);
            j.th = std::min(tile, height - ty);
            jobs.push_back(std::move(j));
        }

    unsigned nthreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    std::mutex mtx;
    size_t next = 0;

    auto worker = [&]() {
        while (true) {
            size_t idx;
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (next >= jobs.size()) return;
                idx = next++;
            }
            encode_one_tile(jobs[idx], plane, width, qp);
        }
    };

    for (unsigned i = 0; i < nthreads; ++i)
        pool.emplace_back(worker);
    for (auto& t : pool) t.join();

    // concatenate in original order
    for (auto& j : jobs)
        payload.insert(payload.end(), j.payload.begin(), j.payload.end());
}

std::vector<uint8_t> encode_frame(const std::vector<uint16_t>& pixels,
                                  int width, int height, int qp, int bit_depth,
                                  int channels) {
    FrameHeader h{};
    h.width     = static_cast<uint16_t>(width);
    h.height    = static_cast<uint16_t>(height);
    h.qp        = static_cast<uint8_t>(qp);
    h.bit_depth = static_cast<uint8_t>(bit_depth);
    h.channels  = static_cast<uint8_t>(channels);
    h.tile_size = 64;

    BitstreamWriter writer;
    writer.write_header(h);

    std::vector<uint8_t> payload;
    size_t plane_size = static_cast<size_t>(width) * height;

    for (int c = 0; c < channels; ++c) {
        std::vector<uint16_t> plane(pixels.begin() + c * plane_size,
                                    pixels.begin() + (c + 1) * plane_size);
        encode_plane_parallel(payload, plane, width, height, qp);
    }

    writer.write_bytes(payload.data(), payload.size());
    return writer.data();
}
