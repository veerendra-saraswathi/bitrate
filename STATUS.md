# Bitrate Project Status — POC / MVP / Final
**RCI SoW Problem 4: Low-Bitrate, High-Resolution Image/Video Compression with Low Latency**

**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Encoder platform:** NVIDIA Jetson Orin Nano (preferred; Xavier NX acceptable)  
**Decoder platform:** Linux PC / Windows PC  
**Last updated:** 2026-10-27

## Overall Status

| Stage              | Completeness | Notes |
|--------------------|--------------|-------|
| POC                | 85%          | Perfect reconstruction on KAIST VIS + IR achieved. Core feasibility shown. |
| MVP                | 45%          | Active focus for next 8–10 weeks. |
| Final (Seeker)     | 5%           | Full SoW acceptance. Depends on solid MVP. |

**Current priority:** Finish MVP.

---

## SoW Key Requirements (summary)

| Requirement                  | Target / Constraint                          |
|-----------------------------|----------------------------------------------|
| Input                       | Visible / Infrared, raw, mono                |
| Indicative Resolution       | ~1920 × 1200 (lower is acceptable)           |
| Bit depth                   | 8 / 10 / 12 / 14 / 16-bit mono or colour     |
| Encoder / Decoder           | Pure C/C++, **no external runtime libraries**|
| Encoder platform            | Jetson Orin Nano                             |
| Decoder platform            | Linux / Windows PC                           |
| Encoder latency             | ≤ 2 frames                                   |
| Streams                     | Single stream only                           |
| Output                      | Bitstream + metadata (~128 bytes placeholder)|
| Storage / Transmission      | File + Ethernet (UDP in RCI framework)       |
| Bandwidth                   | Variable / configurable                      |
| Internet / Cloud            | Not required                                 |
| Perfect reconstruction      | **Hard acceptance criterion**                |
| Priority regions            | Desirable (centre / tracked target)          |

---

## 1. POC

**Goal:** Demonstrate technical feasibility — perfect (or near-perfect) reconstruction, basic latency, and rate-distortion behaviour on sample VIS and IR imagery.

### Achieved
- Cascading 5/3 wavelet with exact round-trip
- Entropy coding (zero-run + improved path)
- Perfect reconstruction on KAIST thermal + visible
- Mono / colour / 16-bit support
- Multi-threaded tiles
- UDP framing
- Basic performance baseline (median / p95 / p99)
- Self-contained C/C++

### Still useful for POC closure
- [ ] Documented latency measurement methodology
- [ ] Short written POC summary

**Status: ~85% — treated as essentially complete.**

---

## 2. MVP

**Goal:** Usable, measurable library that runs on Jetson Orin Nano, produces documented quality–bitrate results on representative VIS/IR data, meets the hard latency bound, and can be demonstrated.

### Target deliverables (mapped to SoW)
- [ ] Jetson Orin Nano build + sustained FPS / latency / memory / CPU-GPU utilisation
- [ ] Encoder latency demonstrated ≤ 2 frames under agreed conditions
- [ ] Quality–bitrate curves (PSNR / SSIM + visual) on KAIST (and any RCI samples)
- [ ] Configurable bitrate / QP / profile mechanism
- [ ] Clean C++ API + simple example
- [ ] Bitstream + metadata specification (128-byte placeholder filled)
- [ ] File storage + basic Ethernet/UDP transfer path
- [ ] Reproducible test suite (different bitrates, VIS, IR, latency, integrity)
- [ ] Comparison vs JPEG baseline
- [ ] Short MVP performance report
- [ ] Optional: simple priority-region / ROI quality bias

### Performance evaluation (SoW §5)
- Perfect reconstruction (qp = 1 or equivalent)
- Compression bitrate & ratio
- Reconstruction error / quality
- Encoder & decoder latency
- CPU/GPU and memory utilisation
- Behaviour at different bandwidths
- Bitstream integrity

**Status: ~45%.**  
Tagged: `mvp-v0.1.0`, `mvp-v0.1.1`.

---

## 3. Final (Seeker-Grade)

**Goal:** Full SoW acceptance for missile-seeker use.

### Remaining for Final
- Production rate-control / bandwidth adaptation
- Robustness (packet loss, partial recovery, temperature)
- Formal acceptance tests against all SoW criteria
- Complete documentation package (algorithm design, API, bitstream spec, integration)
- Priority-region support if required
- Full RCI UDP/Ethernet framing integration
- Environmental / qualification support
- Handover package

**Status: ~5%. Out of current scope.**

---

## Clarifications already answered
- Encoder runs on Jetson Orin Nano inside the platform (missile seeker).
- Decoder needed on both ground station and airborne.
- Single stream only.
- Raw mono data for both VIS and IR.
- Perfect reconstruction is a hard acceptance criterion.
- Latency ≤ 2 frames (end-to-end from frame availability to encoded output).
- Metadata: 128 bytes reserved for now; exact fields later.
- Rate-control mechanism: we decide for POC/MVP.
- Priority regions: desirable if feasible.
- No external libraries, no internet/cloud.

---

## Next 8–10 weeks (MVP focus)
With 2–3 engineers (embedded C++ / Jetson + CV/signal-processing):
1. Jetson Orin Nano timing, memory, and latency proof (≤ 2 frames).
2. Quality–bitrate curves + configurable settings.
3. Clean API, bitstream/metadata spec, test suite.
4. MVP report and tag (`mvp-v0.2`).

Update the checkboxes and % numbers in this file as items are completed.