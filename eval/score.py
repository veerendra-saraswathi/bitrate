#!/usr/bin/env python3
"""Turn raw metrics into a 0-100 score."""

def score_run(metrics, profile="NL"):
    """
    metrics keys: mbps, psnr, max_err, encode_ms, bit_exact
    Returns 0-100.
    """
    mbps = metrics.get("mbps", 100)
    psnr = metrics.get("psnr", 0)
    max_err = metrics.get("max_err", 999)
    encode_ms = metrics.get("encode_ms", 999)
    bit_exact = metrics.get("bit_exact", False)

    # Rate score (lower better). Reference targets from D.
    if profile == "L":
        rate_score = max(0, 100 - (mbps - 9.86) * 4)   # 9.86 Mbps = 100
    else:
        rate_score = max(0, 100 - (mbps - 5.47) * 6)   # 5.47 Mbps = 100

    # Quality score
    quality_score = min(100, psnr * 1.5) if psnr < 70 else 100
    if max_err > 1 and profile == "NL":
        quality_score *= 0.7
    if profile == "L" and not bit_exact:
        quality_score = 0

    # Latency score (2 frames @ 30 fps = 66.7 ms)
    latency_score = max(0, 100 - (encode_ms / 66.7) * 50)

    # Exactness
    exact_score = 100 if bit_exact else (50 if max_err <= 1 else 0)

    if profile == "L":
        total = 0.45 * rate_score + 0.25 * quality_score + 0.15 * latency_score + 0.15 * exact_score
    else:
        total = 0.40 * rate_score + 0.30 * quality_score + 0.20 * latency_score + 0.10 * exact_score

    return round(max(0, min(100, total)), 1)
