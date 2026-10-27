## Streaming latency harness (MacBook, Release) – latest

| Frame              | first_packet | tile full | Encode time | Framing | UDP |
|--------------------|--------------|-----------|-------------|---------|-----|
| 640×512 16-bit     | 1.1 ms       | 2.8 ms    | 4.4 ms      | 0.6 ms  | OK  |
| 640×512 RGB        | 0.2–1.0 ms   | ~2 ms     | 10.5 ms     | 1.8 ms  | OK  |
| 1920×1200 8-bit    | 4.5 ms       | 17.5 ms   | **29.8 ms** | 4.0 ms  | OK  |

- 1920×1200 meets the ≤2-frame (100 ms) budget at 20 Hz with margin.
- First packet ready in <5 ms.
- Bitstream sizes unchanged; reconstruction perfect at qp=1.
