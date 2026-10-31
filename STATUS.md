# Bitrate Project Status — POC / MVP / Final

**RCI SoW Problem 4: Low-Bitrate, High-Resolution Image/Video Compression with Low Latency**

**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Encoder platform:** NVIDIA Jetson Orin Nano (preferred; Xavier NX acceptable)  
**Decoder platform:** Linux PC / Windows PC  
**Last updated:** 2026-10-31 16:05 IST

## Overall Status

| Stage              | Completeness | Notes |
|--------------------|--------------|-------|
| POC                | 85%          | Perfect reconstruction on KAIST VIS + IR with Architecture A. Core feasibility shown. |
| MVP                | 65%          | Host RD, latency and integrity evidence for A and B. D reference frozen. Jetson end-to-end and ~2 Mbps target remain open. |
| Final (Seeker)     | 5%           | Full SoW acceptance. Depends on solid MVP. |

**Current priority:** Freeze D as immutable reference. Lock comparison contract. Run bounded refinement of A (lossless / Profile L) and B (near-lossless / Profile NL). Then Jetson validation.

## Architecture Scores (2026-10-31)

| Architecture                  | Score / 100 | Status                  | Thermal (640×512 @ 30 fps)                                      | Notes |
|-------------------------------|-------------|-------------------------|-----------------------------------------------------------------|-------|
| **A** – 5/3 wavelet           | 65          | Production candidate (L) | ~20.3 Mbps lossless, bit-exact                                 | Host latency good. Refine toward ≤12 Mbps. |
| **B1 / B64** – fast DCT + CRC | 62          | Production candidate (NL)| ~7.0 / 6.6 Mbps @ ~43.5 dB, max_err ≥1                         | Current best lossy. Not bit-exact. Refine toward ≤6 Mbps / ~50 dB. |
| **C** – predictive residual   | 35          | **Closed / flop**       | 24–41 Mbps (lossy uncompetitive)                               | QP=1 exact. Hypothesis rejected. Do not revisit. |
| **D** – JPEG-LS (CharLS)      | 72          | **Reference only**      | NEAR=0: 9.86 Mbps bit-exact; NEAR=1: 5.47 Mbps @ 50.3 dB (max_err=1) | Strongest reference. External library. Frozen. |

### Detailed D results (frozen 2026-10-31)

Source: `results/architecture_D_jpegls.csv` (imagecodecs / CharLS)

**Thermal (KAIST IR):**
- NEAR=0: 9.86 Mbps, bit-exact, encode 3.77 ms
- NEAR=1: 5.47 Mbps, 50.32 dB, max_err=1, encode 2.91 ms
- NEAR=2: 3.51 Mbps, 46.1 dB, max_err=2
- NEAR=4: 2.08 Mbps, 41.0 dB, max_err=4

**Visible (KAIST VIS):**
- NEAR=0: 32.16 Mbps, bit-exact, encode 7.44 ms
- NEAR=1: 19.27 Mbps, 49.9 dB, max_err=1
- NEAR=2: 15.63 Mbps, 45.2 dB, max_err=2
- NEAR=4: 12.58 Mbps, 40.2 dB, max_err=4

D beats A on lossless thermal rate (~2× better) and beats B on near-lossless RD (lower rate + higher PSNR at max_err=1). Host latency excellent on all points.

## SoW Key Requirements (summary)

| Requirement                  | Target / Constraint                          |
|-----------------------------|----------------------------------------------|
| Input                       | Raw mono VIS / IR                            |
| Resolution                  | ~1920×1200 indicative; lower OK for MVP     |
| Bit depth                   | 8/10/12/14/16-bit mono or colour             |
| Encoder / Decoder           | Pure C/C++, **no external runtime libraries**|
| Encoder platform            | Jetson Orin Nano                             |
| Decoder platform            | Linux / Windows PC                           |
| Encoder latency             | ≤ 2 frames                                   |
| Streams                     | Single stream only                           |
| Output                      | Bitstream + ~128-byte metadata placeholder   |
| Storage / Transmission      | File + Ethernet (UDP)                        |
| Bandwidth                   | Variable / configurable                      |
| Perfect reconstruction      | **Hard acceptance criterion**                |
| Priority regions            | Desirable                                    |

## Operating Profiles (architectural decision)

Because perfect reconstruction is a hard SoW criterion:

- **Profile L (Lossless):** Bit-exact (max_err=0). Architecture A path. Required unless RCI explicitly accepts otherwise.
- **Profile NL (Near-lossless):** Bounded reconstruction error (max_err ≤ 1 or as approved). Architecture B path. Only if RCI formally accepts non-zero error.

These are named profiles, not merely QP switches.

## Comparison Contract (locked)

Before further refinement, all A/B measurements use:

- Frames: same KAIST thermal + visible 640×512 raw (`data/kaist_thermal.raw`, `data/kaist_visible.raw`)
- Metrics: Mbps @ 30 fps, PSNR, max absolute error, bit-exact flag, host encode time (median)
- Targets: A ≤ 12 Mbps lossless (stretch 9.86); B ≤ 6 Mbps with max_err ≤ 1 and PSNR closer to 50 dB
- D results frozen and immutable as reference baseline
- Separate columns for raw vs integrity-enabled (CRC) where applicable

## 1. POC

**Status: ~85% — essentially complete.**

Achieved with Architecture A:
- Cascading reversible 5/3 wavelet with exact round-trip
- Entropy coding, multi-threaded tiles and UDP framing
- Perfect reconstruction on KAIST thermal + visible
- Self-contained C/C++

## 2. MVP

**Status: ~65%.**

Host evidence exists for A and B. D reference frozen. Remaining: bounded A/B refinement under the locked contract, Profile L/NL decision, Jetson latency/memory/FPS proof, and approach to the ~2 Mbps target (or documented gap).

### Architecture A (Profile L candidate)
- Thermal lossless: ~20.3 Mbps, bit-exact
- Host warm encode: ~0.86 frames @ 30 fps
- Gaps: rate ~2× worse than D; corruption recovery weaker than B

### Architecture B1/B64 (Profile NL candidate)
- Thermal: ~7.0 / 6.6 Mbps @ ~43.5 dB (32×32 / 64×64 CRC)
- Host encode: ~1.8 frames @ 30 fps
- Gaps: not bit-exact; rate still above D NEAR=1; no Jetson numbers

### Architecture C
Closed. Hypothesis rejected. Do not reopen.

### Architecture D
Reference only. Frozen. Not a production dependency.

## 3. Final (Seeker-Grade)

**Status: ~5%.** Out of current scope until MVP closes.

## Next actions (MVP closure path)

1. Freeze D results permanently (`results/frozen_D_2026-10-31.csv` + checksum).
2. Write `docs/COMPARISON_CONTRACT.md` locking frames, metrics, targets, and timing protocol.
3. **Track A (lossless refinement)** — time-box 2 iterations:
   - Entropy coder alternatives, wavelet levels, tile size
   - Success: ≤12 Mbps thermal lossless, bit-exact
   - Stop if two iterations show no meaningful gain
4. **Track B (near-lossless refinement)** — time-box 2 iterations:
   - QP lookup, residual coding, quantisation
   - Success: ≤6 Mbps with max_err ≤ 1 and higher PSNR
   - Stop if it cannot approach D’s RD
5. Publish **Architecture Selection v1** (A vs B vs D reference) under Profile L / Profile NL.
6. Update this STATUS.md and scores only when measured gates move.
7. Jetson Orin Nano validation (encode/decode latency, memory, sustained FPS, end-to-end ≤2 frames) for the selected pure C++ implementation(s).

**Do not** introduce new transforms or predictors without evidence.  
**Do not** claim the 2 Mbps or Jetson gates closed until measured.

---

*D results source: imagecodecs (CharLS) on 640×512 KAIST frames, 2026-10-31. A/B numbers from 29–30 Oct checkpoint. C closed 29 Oct.*
