# Bitrate Project Status — POC / MVP / Final

**RCI SoW Problem 4: Low-Bitrate, High-Resolution Image/Video Compression with Low Latency**

**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Encoder platform:** NVIDIA Jetson Orin Nano (preferred; Xavier NX acceptable)  
**Decoder platform:** Linux PC / Windows PC  
**Last updated:** 2026-11-02 00:05 IST

## Overall Status

| Stage          | Completeness | Notes |
|----------------|--------------|-------|
| POC            | 90%          | Perfect reconstruction on KAIST VIS + IR with Architecture A. Core feasibility shown. |
| MVP            | 75%          | Matched Jetson A/B1/D measurements complete. D is the clear rate/latency leader on thermal. Pure C/C++ port of D remains open. |
| Final (Seeker) | 8%           | Full SoW acceptance. Depends on solid MVP + pure C/C++ JPEG-LS. |

**Current priority:** Freeze A/B1/D Jetson baselines. Promote D (JPEG-LS) as lead candidate. Retain A as exact-reconstruction reference. Begin pure C/C++ JPEG-LS port feasibility.

## Architecture Scores (2026-11-02, Jetson-validated)

| Architecture                  | Score / 100 | Status                          | Thermal (640×512 @ 30 fps)                  | Notes |
|-------------------------------|-------------|---------------------------------|---------------------------------------------|-------|
| **D** – JPEG-LS (CharLS)      | 78          | **Lead candidate**              | NEAR=4: 2.08 Mbps @ 41.0 dB, 1.86 ms       | Best thermal rate/quality/latency balance. External library today. |
| **A** – 5/3 wavelet           | 64          | Exact-reconstruction reference  | ~20.3 Mbps lossless, ~4 ms, bit-exact      | Excellent latency and exactness. Rate far from 2 Mbps. |
| **B1** – fast DCT + CRC       | 58          | Optimisation candidate          | ~6.6 Mbps @ 43.5 dB, ~119 ms               | Best custom RD. Latency fails SoW. |
| **C** – predictive residual   | 35          | **Closed / flop**               | 24–41 Mbps                                  | Do not revisit. |

### Jetson results summary (Orin Nano Super class, 2026-11-01)

**Architecture A (wavelet)**  
- Thermal 640×512: encode median ~4.0–4.4 ms across QP 1–32; best lossy ~20.3 Mbps @ 24.8 dB.  
- Visible 640×512: encode median ~4.4–6.2 ms; best lossy ~38.6 Mbps.  
- QP=1 bit-exact. Meets ≤2-frame latency easily.

**Architecture B1 (DCT)**  
- Thermal: ~6.6 Mbps @ 43.5 dB (QP=32), encode ~119 ms.  
- Visible: ~9.7 Mbps @ 34.5 dB, encode ~120 ms.  
- Superior RD to A, but latency fails both 33 ms and 66.7 ms budgets. Not bit-exact.

**Architecture D (JPEG-LS / CharLS)** — this Jetson  
| Frame   | NEAR | Mbps @30fps | PSNR  | Encode median | Max err |
|---------|------|-------------|-------|---------------|---------|
| Thermal | 0    | 9.86       | 99.0 | ~10.8 ms     | 0      |
| Thermal | 1    | 5.47       | 50.3 | ~5.4 ms      | 1      |
| Thermal | 2    | 3.51       | 46.1 | ~3.2 ms      | 2      |
| Thermal | 4    | **2.08**   | **41.0** | **1.86 ms** (p95 1.88) | 4 |
| Visible | 4    | 12.58      | 40.2 | ~17 ms       | 4      |

D is the first architecture that simultaneously approaches the 2 Mbps thermal target, retains usable quality, and stays well inside the Jetson latency budget.

## SoW Key Requirements (summary)

| Requirement            | Target / Constraint                           |
|------------------------|-----------------------------------------------|
| Input                  | Raw mono VIS / IR                             |
| Resolution             | ~1920×1200 indicative; lower OK for MVP       |
| Bit depth              | 8/10/12/14/16-bit mono or colour              |
| Encoder / Decoder      | Pure C/C++, **no external runtime libraries** |
| Encoder platform       | Jetson Orin Nano                              |
| Decoder platform       | Linux / Windows PC                            |
| Encoder latency        | ≤ 2 frames                                    |
| Perfect reconstruction | Hard acceptance criterion (Profile L)         |
| Bandwidth              | Variable / configurable; ~2 Mbps thermal goal |

## Operating Profiles

- **Profile L (Lossless):** Bit-exact (max_err=0). Architecture A remains the reference.  
- **Profile NL (Near-lossless):** Bounded error. Architecture D (NEAR≥1) is the new lead path; B1 is secondary.

## Comparison Contract (locked)

- Frames: same KAIST thermal + visible 640×512 raw  
- Metrics: Mbps @ 30 fps, PSNR, max absolute error, bit-exact flag, encode median/p95  
- Platforms: host + Jetson Orin Nano class  
- D CharLS numbers frozen as reference; production path must be pure C/C++

## Next actions (MVP closure path)

1. Freeze A/B1/D Jetson baselines permanently (`results/frozen_jetson_2026-11-01.csv` + checksum).  
2. CharLS license & dependency review (BSD-3-Clause).  
3. **Pure C/C++ JPEG-LS port feasibility** (priority):  
   - Lossless (NEAR=0) first, then NEAR=1/2/4.  
   - Match CharLS bitrate/PSNR on the same KAIST frames.  
   - No external runtime library.  
4. Decoder timing + repeated-run distributions for the pure port.  
5. Thermal NEAR=4 refinement (push 2.08 Mbps under 2 Mbps if possible).  
6. Visible compression investigation (secondary).  
7. Architecture Selection v2 once pure D port numbers exist.  
8. Jetson re-validation of the pure C/C++ D implementation.

**Do not** invest further in B1 SIMD until the pure JPEG-LS port feasibility is known.  
**Do not** claim the 2 Mbps or pure-C++ gates closed until measured on the self-contained implementation.

---

*Jetson measurements: EDGE AI Platform Orin Nano Super class, 2026-11-01.  
A/B1/D numbers from matched KAIST 640×512 frames. C remains closed.*
