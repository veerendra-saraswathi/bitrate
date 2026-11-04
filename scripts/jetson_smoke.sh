#!/bin/bash
# Jetson smoke test for Profile L (A) and Profile NL (B)
# Usage: ./scripts/jetson_smoke.sh

set -e
FRAME=data/kaist_thermal.raw
W=640
H=512

echo "=== Profile L (A, mode 1, QP=1) ==="
/usr/bin/time -f "encode_time %e" ./encoder $FRAME $W $H 1
ls -l encoded.bin
./decoder encoded.bin $FRAME
echo

echo "=== Profile NL (B) — run your DCT binary here ==="
# ./dct_encoder $FRAME $W $H --qp 1
# ./dct_decoder ...
echo "(fill in actual B binary names)"
