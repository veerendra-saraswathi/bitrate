# Architecture Ratings and Completion Status

Last updated: 2026-11-07 (Asia/Kolkata)

## Architecture ratings (current)

Scores are provisional where noted. Host evidence only unless stated otherwise.  
Jetson validation and full native-14-bit locked KAIST comparison remain open for all entries.

| Rank | Architecture | Score /100 | Status | Detailed remarks |
|------|--------------|------------|--------|------------------|
| 1 | **E – Final ewc codec** | **82** | Provisional (host) | Single pure-C++ binary that selects among D+, wavelet, near-lossless and two-layer paths. Strong rate control, CRC resilience, documented 2 Mbps results. Best overall package against the internal rubric. |
| 2 | E – wavelet, tiled (PoC) | 80 | Provisional | Best average quality at the 2 Mbps point (highest PSNR/SSIM/MAE on the reported frames). Exact per-frame byte budget. Tiled for damage isolation. |
| 3 | E – D+ (ewc arithmetic) | 79 | Provisional | Best lossless and near-lossless performance; consistently smaller than CharLS on the tested points. Exact ±N error control. |
| 4 | E – Two-layer (wavelet + D+) | 77 | Provisional | Guarantees a maximum error bound at 2 Mbps while staying close to pure-wavelet quality. Useful resilience property if the residual layer is lost. |
| 5 | E – near-lossless (Golomb) | 74 | Provisional | Simple and fast. Exact ±N control. Weaker compression than the arithmetic D+ path. |
| 6 | **D** | **72** | Reference | CharLS / JPEG-LS. Strong, well-characterised lossless baseline. Used as the reference for D+ comparisons. |
| 7 | **A** | **65** | Active | Reversible 5/3 wavelet. Bit-exact path works. Rate figures need configuration-matched reconfirmation against earlier reports. |
| — | B | — | Historical | DCT baseline. Retained for reference; no new development. |
| — | C | — | Stopped | Predictive residual. Development closed. |
| — | **F** | **Deferred** | Deferred | Exactness closed on tested frames (qstep=1 bit-exact after int32 path). 8,333-byte budget not met; rate floor explained by Rice + block overhead. Deferred for the one-week review. Code and diagnostics retained. |

## Completion status (PoC, MVP, Final solution)

Completion percentages are **Not assessed**.

Reason: stage checklists and weights have not been frozen.  
Per the evaluation rules, no numeric percentage is assigned until the checklists are defined and evidence is reviewed against them.

| Stage | Completion | Notes |
|-------|------------|-------|
| PoC | Not assessed | Host rate/quality evidence exists for E, but native 14-bit ingest/output, Jetson latency, and stored-stream damage tests are incomplete. |
| MVP | Not assessed | Product scope, profiles/API, and repeatable target benchmarks not frozen. |
| Final solution | Not assessed | SoW acceptance criteria, target qualification, robustness package, and release documentation not closed. |

An architecture score (for example 82/100) is a technical suitability rating.  
It is not a completion percentage.

## Summary

- Strongest current candidate: **Architecture E (final ewc codec)** at 82/100 provisional.
- Architecture F is deferred after exactness was closed and the 2 Mbps budget was shown to be unmet.
- No PoC / MVP / Final completion percentage is claimed until checklists are agreed.

See also [INTERNAL_EVALUATION.md](INTERNAL_EVALUATION.md).
