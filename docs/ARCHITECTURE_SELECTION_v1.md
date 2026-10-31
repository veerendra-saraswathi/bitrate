# Architecture Selection v1 (2026-11-01)

## Decision
- **Profile L (bit-exact):** Architecture A (mode 1, levels=1).  
  Thermal ~59 Mbps host (or ~20 Mbps from earlier checkpoint — confirm which number is canonical).  
  Only pure-C++ path that meets max_err=0.
- **Profile NL (near-lossless):** Architecture B1 (QP=1, 32×32 CRC).  
  Thermal 14.0 Mbps / 67.6 dB / max_err=1.  
  Best pure-C++ lossy point measured so far.
- **Reference only:** Architecture D (frozen).  
  D0 lossless 9.86 Mbps, D1 NEAR=1 5.47 Mbps @ 50.3 dB.

## Leaderboard (thermal 640×512, host)
| Config       | Score | Mbps  | PSNR | max_err | Role          |
|--------------|-------|-------|------|---------|---------------|
| D0_lossless  | 99.6  | 9.86  | ∞    | 0       | Reference     |
| D1_near1     | 87.2  | 5.47  | 50.3 | 1       | Reference     |
| B1_qp1       | 63.3  | 14.01 | 67.6 | 1       | Profile NL    |
| A1_mode1     | 47.5  | 59.46 | ∞    | 0       | Profile L     |

## Open items before Jetson
- Confirm canonical A rate (earlier checkpoint ~20 Mbps vs current harness 59 Mbps).
- No further host refinement planned unless a clear <1-day win appears.
- Jetson will measure encode/decode latency, memory, and sustained FPS for the selected A and B binaries.
