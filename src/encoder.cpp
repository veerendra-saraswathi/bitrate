#include "bitstream.h"
#include "wavelet.h"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <atomic>

struct TileTiming {
    double first_packet_ms = 0;
    double full_ms = 0;
    std::vector<double> tile_ms;
};
static TileTiming g_timing;
static std::mutex g_timing_mtx;

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
            else out.push_back(0);
            write_val(out, c);
        }
    }
    if (run) out.push_back(static_cast<uint8_t>(run));
}

struct TileJob {
    int index;
    int ty, tx, tw, th;
    std::vector<uint8_t> payload;
    bool done = false;
};

static void encode_one_tile(TileJob& job, const std::vector<uint16_t>& plane, int width, int qp) {
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<int32_t> t(job.tw * job.th);
    for (int y = 0; y < job.th; ++y)
        for (int x = 0; x < job.tw; ++x)
            t[y * job.tw + x] = plane[(job.ty + y) * width + (job.tx + x)];
    wavelet_5_3_forward(t, job.tw, job.th);
    quantise(t, qp);
    rice_encode(job.payload, t);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    {
        std::lock_guard<std::mutex> lock(g_timing_mtx);
        g_timing.tile_ms.push_back(ms);
    }
}

static void encode_plane_streaming(std::vector<uint8_t>& payload,
                                   const std::vector<uint16_t>& plane,
                                   int width, int height, int qp) {
    const int tile = 64;
    std::vector<TileJob> jobs;
    int idx = 0;
    for (int ty = 0; ty < height; ty += tile)
        for (int tx = 0; tx < width; tx += tile)
            jobs.push_back({idx++, ty, tx,
                            std::min(tile, width - tx),
                            std::min(tile, height - ty), {}, false});

    auto t_start = std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::mutex> lock(g_timing_mtx);
        g_timing.tile_ms.clear();
    }

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<size_t> next_job{0};
    unsigned nthreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;

    auto worker = [&]() {
        while (true) {
            size_t i = next_job.fetch_add(1);
            if (i >= jobs.size()) return;
            encode_one_tile(jobs[i], plane, width, qp);
            {
                std::lock_guard<std::mutex> lock(mtx);
                jobs[i].done = true;
            }
            cv.notify_all();
        }
    };

    for (unsigned t = 0; t < nthreads; ++t) pool.emplace_back(worker);

    size_t emitted = 0;
    bool first = false;
    while (emitted < jobs.size()) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]{ return jobs[emitted].done; });
        payload.insert(payload.end(),
                       jobs[emitted].payload.begin(),
                       jobs[emitted].payload.end());
        if (!first) {
            first = true;
            g_timing.first_packet_ms =
                std::chrono::duration<double, std::milli>(
                    std::chrono::high_resolution_clock::now() - t_start).count();
        }
        ++emitted;
    }
    for (auto& t : pool) t.join();

    auto t_end = std::chrono::high_resolution_clock::now();
    g_timing.full_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    std::lock_guard<std::mutex> lock(g_timing_mtx);
    if (!g_timing.tile_ms.empty()) {
        std::sort(g_timing.tile_ms.begin(), g_timing.tile_ms.end());
        double med = g_timing.tile_ms[g_timing.tile_ms.size()/2];
        std::cout << "Tiles: " << g_timing.tile_ms.size()
                  << "  first_packet=" << g_timing.first_packet_ms << " ms"
                  << "  median_tile=" << med << " ms"
                  << "  full=" << g_timing.full_ms << " ms\n";
    }
}

std::vector<uint8_t> encode_frame(const std::vector<uint16_t>& pixels,
                                  int width, int height, int qp, int bit_depth,
                                  int channels) {
    FrameHeader h{};
    h.width = static_cast<uint16_t>(width);
    h.height = static_cast<uint16_t>(height);
    h.qp = static_cast<uint8_t>(qp);
    h.bit_depth = static_cast<uint8_t>(bit_depth);
    h.channels = static_cast<uint8_t>(channels);
    h.tile_size = 64;

    BitstreamWriter writer;
    writer.write_header(h);

    std::vector<uint8_t> payload;
    size_t plane_size = static_cast<size_t>(width) * height;
    for (int c = 0; c < channels; ++c) {
        std::vector<uint16_t> plane(pixels.begin() + c * plane_size,
                                    pixels.begin() + (c + 1) * plane_size);
        encode_plane_streaming(payload, plane, width, height, qp);
    }
    writer.write_bytes(payload.data(), payload.size());
    return writer.data();
}
