# Internal Architecture Evaluation and Delivery Progress

Last updated: 2026-11-07 (Asia/Kolkata)

## Purpose and authority

Track architectures A–F against the same SoW-aligned internal rubric, and maintain
separate evidence-based completion trackers for PoC, MVP and the final solution.
Update this file as results become available; retain the assessment history.

These weights are internal, not customer-issued acceptance criteria. The signed
SoW and agreed clarifications take precedence. A score cannot compensate for a
failed mandatory requirement. Unverified claims are not passes.

## Evaluation parameters

| Evaluation parameter | Maximum points | Evidence required |
| --- | ---: | --- |
| Low bitrate / rate control | 25 | Useful operation near 2 Mbps; actual bytes/frame and Mbps at the stated fps; include applicable headers, metadata and protection overhead |
| Reconstruction quality / detail | 20 | PSNR with declared sample range, RMSE, maximum error, structural quality and visual inspection |
| Encoder and decoder latency | 20 | Encoder ≤2 frame periods; median, p95/p99 and maximum; decoder latency and sustained throughput |
| Native input/output correctness | 10 | 640×512, native 14-bit monochrome IR; correct raw output as two bytes/sample, row-major; input-range validation |
| Bitstream, metadata and decoder | 10 | Compatible decoder, documented/versioned format, metadata round-trip and file storage |
| Corruption handling / recovery | 5 | Reproducible truncation, bit-error and loss tests; detection, recovery, propagation and decoder stability |
| Self-contained C/C++ | 5 | Dependency audit against the agreed restriction; distinguish external runtime dependencies from bundled third-party code |
| Reproducibility / evidence | 5 | Frozen commit, input hashes, configurations, build instructions, raw results and hardware details |
| **Total** | **100** | |

## Measurement contract

- Use the locked KAIST thermal and visible frames for continuity; also evaluate
  representative native 14-bit IR sequences. Stretched 8-bit input is supplementary.
- Record input hashes, dimensions, bit depth, byte order, preprocessing, frame rate,
  source commit, profile, parameters, compiler, build flags and hardware.
- Report total codec bytes separately from transport/FEC bytes. State whether the
  rate limit applies to the stored stream or the complete transmitted stream.
- At an assumed 30 fps, 2 Mbps permits floor(2,000,000 / 8 / 30) = 8,333 bytes/frame;
  two frame periods are 66.67 ms. These assumptions require customer confirmation.
- Measure latency from the agreed input availability boundary to complete encoded
  output. Include rate-control searches. Report throughput and latency separately.
- Report host and target results separately. Record Jetson model, power mode,
  clocks, thread count and sustained-run conditions; do not mix platforms in a score.
- Lossless PSNR is mathematically infinite. Label numeric display sentinels explicitly.
- Failed reconstruction or missing metrics must be reported as invalid/N/A, not zero.
- Exclude results from non-round-tripping lossless backends from valid comparisons.

See [COMPARISON_CONTRACT.md](COMPARISON_CONTRACT.md) for the existing internal
lossless/near-lossless comparison. The approximately 2 Mbps PoC evaluation is
a separate operating point.

## Operating modes

| Mode | Objective | Boundary |
| --- | --- | --- |
| PoC fixed-rate lossy | Useful quality near 2 Mbps on native 14-bit IR | Quality thresholds, fps, rate interpretation and loss model require agreement |
| Profile L | Bit-exact reconstruction, max_err = 0; internal thermal target ≤12 Mbps | Internal KAIST comparison target; not a native 14-bit acceptance claim |
| Profile NL | Verified max_err ≤1; internal thermal target ≤6 Mbps | Internal KAIST comparison target; coefficient quantisation alone does not prove this bound |

Score each mode separately. Do not combine the best results from different modes
as though one operating point achieved them simultaneously.

## Architecture assessment register

`Pending` means no assessment has been entered under this rubric; it is not zero.
Existing leaderboard scores and earlier provisional reviews are not automatically
transferred into this register.

| Architecture | Implementation / reference | Profile and tested commit | Score /100 | Mandatory gates | Evidence / next action |
| --- | --- | --- | --- | --- | --- |
| A | Reversible 5/3 baseline | Pending | Pending | Unverified here | Reconcile historical rate differences; use current native-u16 implementation |
| B | DCT baseline | Pending | Pending | Unverified here | Retain historical results with configuration and hardware attribution |
| C | Predictive residual baseline; development stopped | Pending | Pending | Unverified here | Retain historical evidence; no new development implied |
| D | JPEG-LS / CharLS reference | Pending | Pending | Unverified here | Preserve frozen reference; record dependency-policy interpretation |
| E | bitrate2; multiple coding paths (final ewc codec) | Pending | **82 (provisional)** | Jetson unverified | Host evidence strong; assess paths separately; target-hardware validation open |
| F | experiments/F; V5 + int32 lossy + zero-run | Current | **Deferred** | Rate budget unmet | Exactness closed on tested frames; 2 Mbps budget not met; rate floor documented. Deferred for review. |

For every new scored assessment, append a row below and link the raw evidence.
Assign integer points from 0 to each category maximum with a written rationale.
Give unverified criteria no awarded points in a provisional numeric total, label
them unverified (not failed), and state evidence coverage. Record partial credit
explicitly. Thresholds and partial-credit rules must be frozen before using scores
for a selection decision.

| Date | Architecture / mode | Commit / dataset / platform | Rate /25 | Quality /20 | Latency /20 | I/O /10 | Stream /10 | Recovery /5 | C++ /5 | Evidence /5 | Total /100 | Gate status / rationale / evidence |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 2026-11-07 | E / final ewc codec (host) | bitrate2 | 22 | 18 | 16 | 8 | 8 | 4 | 5 | 1 | **82** | Provisional. Strong host rate/quality/resilience. Jetson and full native-14-bit locked KAIST unverified. |
| 2026-11-07 | F / lossy (host) | experiments/F | — | — | — | — | — | — | — | — | Deferred | Exactness resolved (qstep=1 bit-exact on tested frames). 8333-byte budget unmet. Rate floor explained. Deferred. |

## Completion tracking: PoC, MVP and final solution

Architecture score measures technical suitability. Completion percentage measures
verified delivery against a defined checklist. An 82/100 score is not 82% complete.
Do not average architecture completion values into overall project completion.

Before reporting a percentage, freeze a separate deliverable checklist and weights
for each stage. Use `Open`, `In progress`, `Blocked`, or `Verified complete` for
each item. Award completion weight only when its evidence and review are recorded.

Completion (%) = 100 × verified-complete deliverable weight / total agreed stage weight.

If the checklist or weights are not agreed, report `Not assessed`, not an invented
percentage. Record scope changes and recompute the denominator transparently.

| Stage | Completion | Checklist / weights | Evidence and remaining gates |
| --- | --- | --- | --- |
| PoC | Not assessed | To be agreed | Native 14-bit ingest/output, metadata/storage, quality near target rate, latency and stored-stream damage evaluation |
| MVP | Not assessed | To be agreed | Freeze product scope, profiles/API, repeatable tests, target benchmarks and known limitations |
| Final solution | Not assessed | To be agreed against final SoW | Target qualification, required formats/endpoints, robustness, release documentation and customer acceptance |

Deliverable checklist template (copy separately for each stage):

| ID | Deliverable / exit criterion | Weight | Status | Evidence link / commit | Reviewer / date |
| --- | --- | ---: | --- | --- | --- |
| — | Define and agree stage checklist before calculating completion | — | Open | — | — |

## Update procedure

1. Freeze the tested code, input data and operating mode.
2. Attach raw correctness, rate, quality, latency and robustness evidence.
3. Record mandatory gates as Pass, Fail or Unverified, with reasons.
4. Enter per-category scores and rationale; update the architecture register.
5. Update verified stage deliverables and recalculate completion separately.
6. Commit the file and evidence references; retain previous assessment rows.

## History

| Date | Change |
| --- | --- |
| 2026-11-07 | Established internal 100-point rubric, architecture register and separate completion method. No historical score or completion estimate adopted as verified. |
| 2026-11-07 | Added provisional score for E (final ewc codec, 82). Marked F deferred (exactness closed, 2 Mbps budget unmet). |

## Landing-page note

This file is the current internal evaluation landing page for the bitrate repository.
See also the main [README.md](../README.md).
