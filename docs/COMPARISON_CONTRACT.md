# Comparison Contract (locked 2026-10-31)

All Architecture A and B refinement measurements must follow this contract.
Architecture D results are frozen and immutable.

## Frames
- Thermal: `data/kaist_thermal.raw` (640×512, 8-bit mono)
- Visible: `data/kaist_visible.raw` (640×512, 8-bit mono)

## Metrics (report all of these)
- Bitrate: Mbps at 30 fps
- Quality: PSNR (dB)
- Error: max absolute reconstruction error
- Exactness: bit-exact flag (max_err == 0)
- Latency: host encode time in ms (median of ≥10 runs after one warm-up)

## Success targets
| Profile | Architecture | Thermal target | Constraints |
|---------|--------------|----------------|-------------|
| L (Lossless) | A | ≤ 12 Mbps | max_err = 0 (bit-exact) |
| NL (Near-lossless) | B | ≤ 6 Mbps | max_err ≤ 1, higher PSNR preferred |

Stretch targets (D reference):
- Lossless thermal: 9.86 Mbps
- Near-lossless thermal: 5.47 Mbps @ ~50.3 dB (max_err=1)

## Rules
- D results (`results/frozen_D_2026-10-31.csv`) are immutable reference only.
- Report raw and integrity-enabled (CRC) numbers separately when applicable.
- Do not change the frames or metric definitions during refinement.
- Host measurements only until Jetson is available.
- Any configuration that violates max_err constraints is rejected.

## Timing protocol
1. Warm-up run (discard)
2. ≥10 timed runs
3. Report median encode time

