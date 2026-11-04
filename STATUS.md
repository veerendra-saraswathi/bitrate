# Bitrate Project Status — POC / MVP / Final

**RCI SoW Problem 4: Low-Bitrate, High-Resolution Image/Video Compression with Low Latency**

**Repo:** https://github.com/veerendra-saraswathi/bitrate  
**Encoder platform:** NVIDIA Jetson Orin Nano (preferred; Xavier NX acceptable)  
**Decoder platform:** Linux PC / Windows PC  
**Last updated:** 2026-11-03 21:40 IST

## Overall Status

## MVP Checklist (2026-11-05 02:38 IST)

| Item | Status | Notes |
|------|--------|-------|
| Perfect reconstruction (A QP=1, D NEAR=0) | Done | Host, 8/10/12/14/16-bit |
| Public API round-trip | Done | encode/decode dispatch |
| 16-bit path (A and D) | Done | Host verified |
| 8/10/12/14/16-bit mono | Done | Host verified |
| Jetson latency ≤ 2 frames | Done | Frozen baseline (A and D) |
| External-library wording | Open | Awaiting owner confirmation |
| Metadata definition | Open | Placeholder documented; owner input needed |
| ~1920×1200 resolution | Open | Proxy only (480×640 / 640×512) |
| Bitrate / quality thresholds | Open | Not yet agreed |
| Ethernet / UDP transfer | Open | Not in current MVP scope |
| Linux/Windows decoder verification | Open | Mac only so far |
| Pure C/C++ JPEG-LS port | Deferred | Only if owner rejects static CharLS |

MVP completeness (technical core): ~80 %.  
Remaining blockers are owner decisions and integration items.



| Stage          | Completeness | Notes |
|----------------|--------------|-------|
| POC            | 90%          | Perfect reconstruction on KAIST VIS + IR with Architecture A. Core feasibility shown. |
| MVP            | 82%          | Public API encode+decode for Architecture D (vendored static CharLS) verified on KAIST thermal (NEAR=0 bit-exact 41067 B; NEAR=4 max_err=4, 8656 B). Jetson baselines frozen. Pure C/C++ port deferred post-MVP. |
| Final (Seeker) | 8%           | Full SoW acceptance. Depends on solid MVP + pure C/C++ JPEG-LS. |

**Current priority:** Record dependency interpretation (vendored static CharLS). Keep A as exact reference. Next: Jetson smoke of the public D path, then rate-control / remaining SoW acceptance items. Pure C/C++ port is post-MVP.

## Architecture Scores (2026-11-02, Jetson-validated)

| Architecture                  | Score / 100 | Status                          | Thermal (640×512 @ 30 fps)                  | Notes |
|-------------------------------|-------------|---------------------------------|---------------------------------------------|-------|
| **D** – JPEG-LS (CharLS)      | 82          | **Lead candidate (MVP)**        | NEAR=4: 2.08 Mbps @ 41.0 dB, 1.86 ms       | Vendored static CharLS. Public API round-trip green. Pure port deferred. |
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

### Host (MacBook) confirmation — Architecture D (2026-11-02)

Architecture D’s JPEG-LS benchmark builds and runs on the MacBook. On the selected KAIST thermal input, NEAR=0 produces 41,067 bytes with max_err=0 and median encode time 21.23 ms; NEAR=4 produces 8,656 bytes with max_err=4 and median encode time 6.99 ms. Compressed sizes reproduce the frozen baseline. Integration through the main encode_frame API and final SoW compliance remain unverified.

Keep the existing Jetson measurements frozen. A Mac executable would require a compatible rebuild for Jetson.

A 16-bit stub would not demonstrate higher-bit-depth support, and the build log alone does not prove absence of external runtime dependencies. These claims should remain pending verification.

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

## Dependency interpretation (open)

SoW text: “Pure C/C++, no external runtime libraries.”

| Interpretation | Meaning | Current CharLS status | Acceptable? |
|----------------|---------|-----------------------|-------------|
| No external **runtime** dependency | Binary does not require a separate shared library at run time | Satisfied — CharLS is statically linked (source compiled in) | Likely yes |
| No third-party libraries at all | No external source, even if statically linked | Not satisfied — CharLS source is still third-party | Unknown |
| “Pure C++” | Implementation language only | Satisfied | Does not decide the above |

**Decision needed:** Does “no external runtime libraries” allow a statically linked, BSD-licensed third-party codec (CharLS), or does it require a from-scratch implementation?

Until that is clarified:
- Do not start a pure-port effort.
- Do not re-measure on Jetson.
- Do not raise the MVP completion percentage.

Next action: obtain or record the accepted interpretation, then either (a) lock the vendored-CharLS path or (b) begin a minimal pure-C++ JPEG-LS port.

## Dependency interpretation (open)

SoW text: “Pure C/C++, no external runtime libraries.”

| Interpretation | Meaning | Current CharLS status | Acceptable? |
|----------------|---------|-----------------------|-------------|
| No external **runtime** dependency | Binary does not require a separate shared library at run time | Potentially satisfied for CharLS; verify the executable’s dependencies | Subject to customer acceptance |
| No third-party libraries at all | No external source, even if statically linked | Not satisfied — CharLS source is still third-party | Unknown |
| “Pure C++” | Implementation language only | Satisfied | Does not decide the above |

**Precise clarification to record:**

Does the requirement permit third-party C/C++ source compiled and statically linked into the delivered executable, provided no separate codec runtime library is required?

- Working interpretation: static linking may comply.
- Accepted interpretation: not yet documented.

The wording supports “no separately required codec library at runtime,” rather than an automatic ban on all third-party source. But we have no recorded RCI confirmation that this is the accepted contractual interpretation.

For the quoted line—“Pure C/C++, no external runtime libraries”:
- Pure C/C++ specifies the implementation language.
- No external runtime libraries ordinarily concerns dependencies required when executing the delivered software.
- Statically linked CharLS remains third-party code, but would not require a separate CharLS shared library at runtime.

Therefore, a from-scratch implementation is not explicitly required by that sentence alone. Another SoW clause could impose stricter restrictions.

Until the accepted interpretation is documented:
- Do not start a pure-port effort.
- Do not re-measure on Jetson.
- Do not raise the MVP completion percentage.

## Runtime dependency check (Mac, 2026-11-03)

Command: `otool -L test_dispatch` / `otool -L bench_jpegls`

Result: [paste the output here]

Conclusion: CharLS is / is not present as a separate shared library.  
This confirms the “no external runtime dependency” working interpretation for the current static build.  
Accepted contractual interpretation remains undocumented.

## Architecture D dependency (2026-11-03)
Architecture D uses vendored CharLS source, compiled and statically linked into bitrate_core.
otool -L on the resulting binaries shows only macOS libc++ and libSystem; no separate CharLS shared library is required at runtime.
This is the current MVP approach. Whether static linking of third-party source fully satisfies the SoW phrase “Pure C/C++, no external runtime libraries” remains an interpretation to be confirmed. Attribution (BSD-3-Clause) is retained under third_party/.

Public API encode + decode round-trip for Architecture D is now verified:
- NEAR=0: bit-exact (max_err=0), 41067 bytes on KAIST thermal 640×512
- NEAR=4: max_err=4, 8656 bytes (matches frozen reference)

## Extreme-case 16-bit host result (2026-11-05 00:41 IST)

- Source: LTIR 16-bit frame (16_jacket, 640×480, range 0–17070)
- Public API extension: `encode_frame_u16` / `decode_frame_u16` (Architecture D)
- NEAR=0: 308324 bytes, max_err=0 (bit-exact)
- NEAR=4: 186764 bytes, max_err=4
- 8 additional LTIR 16-bit frames selected under `data/extreme/ltir/selected/`
- Limitation: resolution is 480×640 (not the SoW indicative ~1920×1200). Bit-depth extreme case is now verified on host.

## Extreme-case 16-bit host result — Architecture A (2026-11-05 01:21 IST)

- Source: same LTIR 16-bit frame (16_jacket, 640×480, range 0–17070)
- Public API extension: `encode_frame_a_u16` / `decode_frame_a_u16`
- QP=1: 815866 bytes, max_err=0 (bit-exact)
- Architecture A remains the exact-reconstruction reference on 16-bit data.

## SoW acceptance matrix — Architecture A vs D (2026-11-05 01:26 IST)

| Criterion | Architecture A | Architecture D | Evidence level |
|-----------|----------------|----------------|----------------|
| Perfect reconstruction (Profile L) | Met (QP=1, max_err=0 on 8-bit and 16-bit) | Met (NEAR=0, max_err=0 on 8-bit and 16-bit) | Host verified |
| Encoder latency ≤ 2 frames | Met (~4 ms on Jetson, frozen) | Met (~1.9–4 ms on Jetson, frozen) | Frozen Jetson baseline |
| Pure C/C++, no external runtime libraries | Met (self-contained wavelet) | Open (static CharLS; runtime free, source dependency awaits owner confirmation) | Interpretation pending |
| ~1920×1200 indicative resolution | Proxy only (640×512 / 480×640 tested) | Proxy only | Not met |
| 8/10/12/14/16-bit mono | Verified (8/10/12/14/16-bit host, max_err=0) | Verified (8/10/12/14/16-bit host, max_err=0) | Host verified |
| Bitstream + required metadata | Partial (FrameHeader present) | Partial (JPEG-LS markers; custom metadata not defined) | Metadata definition open |
| Variable bandwidth / rate control | Partial (QP sweep) | Partial (NEAR sweep) | Thresholds not agreed |
| Ethernet / UDP transfer | Not demonstrated | Not demonstrated | Open |
| Decoder on Linux/Windows PC | Not verified (Mac only) | Not verified (Mac only) | Open |
| Encoder on designated Jetson | Met (frozen baseline) | Met (frozen baseline) | Frozen |
| Complete source, docs, reports | Partial | Partial | In progress |

## SoW Acceptance Matrix — Architecture A vs D (2026-11-05 01:36 IST)

| Criterion | Architecture A | Architecture D | Evidence level |
|-----------|----------------|----------------|----------------|
| Perfect reconstruction (Profile L) | Met (QP=1, max_err=0 on 8-bit and 16-bit) | Met (NEAR=0, max_err=0 on 8-bit and 16-bit) | Host verified |
| Encoder latency ≤ 2 frames | Met (~4 ms on Jetson, frozen) | Met (~1.9–4 ms on Jetson, frozen) | Frozen Jetson baseline |
| Pure C/C++, no external runtime libraries | Met (self-contained wavelet) | Open (static CharLS; runtime free, source dependency awaits owner confirmation) | Interpretation pending |
| ~1920×1200 indicative resolution | Proxy only (640×512 / 480×640 tested) | Proxy only | Not met |

| Bitstream + required metadata | Partial (FrameHeader present) | Partial (JPEG-LS markers; custom metadata not defined) | Metadata definition open |
| Variable bandwidth / rate control | Partial (QP sweep) | Partial (NEAR sweep) | Thresholds not agreed |
| Ethernet / UDP transfer | Not demonstrated | Not demonstrated | Open |
| Decoder on Linux/Windows PC | Not verified (Mac only) | Not verified (Mac only) | Open |
| Encoder on designated Jetson | Met (frozen baseline) | Met (frozen baseline) | Frozen |
| Complete source, docs, reports | Partial | Partial | In progress |

## Metadata placeholder (2026-11-05 02:37 IST)

Current bitstream contents:

| Architecture | What is present today | What is still open |
|--------------|-----------------------|--------------------|
| A (wavelet) | FrameHeader (width, height, qp, levels, tile_size, bit_depth, channels, flags) | Owner-defined fields (timestamp, sensor ID, frame sequence, ROI, etc.) |
| D (JPEG-LS) | Standard JPEG-LS markers (SOI, SOF55, SOS, EOI) + frame dimensions and bit depth inside SOF | Same owner-defined fields; no custom APPn segment yet |

Status: Partial.  
The bitstream is self-describing for decode. Additional metadata required by the SoW (“required metadata for each image/frame”) is not yet defined by the requirement owner.

