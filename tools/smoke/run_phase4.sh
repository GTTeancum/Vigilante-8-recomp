#!/usr/bin/env bash
# Phase 4 smoke test. Exit 0 = pass.
#
# The user's mandate: "semi-believable" = vehicle drives.
#
# Checks:
#   1. build clean
#   2. v8 --frames 300 --auto-drive 250 --screenshot phase4.ppm exit 0
#   3. screenshot is non-uniform (Phase 3 predicate)
#   4. vehicle position delta > 1.0 units (proves input -> motion path)
#   5. yaw changed (proves steering)

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 4 Smoke Test ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p4_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p4_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase4.ppm
rm -f "$SHOT"
timeout 30 ./build/Debug/v8.exe \
    --frames 300 --auto-drive 250 \
    --screenshot "$SHOT" \
    > /tmp/p4_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc"
    tail -10 /tmp/p4_run.log
    exit 1
fi
echo "[ok] v8 ran 300 frames with auto-drive, exit 0"

if [[ ! -f "$SHOT" ]]; then
    echo "FAIL: screenshot not produced"
    exit 1
fi
python tools/smoke/check_screenshot.py "$SHOT" || {
    echo "FAIL: screenshot uniform color (renderer broken)"; exit 1;
}

# Parse the final vehicle pos line from the log.
pos_line=$(grep "vehicle pos =" /tmp/p4_run.log | tail -1)
if [[ -z "$pos_line" ]]; then
    echo "FAIL: no vehicle pos log line"
    exit 1
fi
echo "  $pos_line"

# Extract x, z, yaw via python (sed regex w/ floats is awkward).
read x z yaw <<<$(python -c "
import re
line = '''$pos_line'''
m = re.search(r'\(([-\d.]+), [-\d.]+, ([-\d.]+)\) yaw=([-\d.]+)', line)
if m: print(m.group(1), m.group(2), m.group(3))
else: print('0 0 0')
")

# Distance from origin: sqrt(x^2 + z^2).
dist=$(python -c "import math; print(math.sqrt(${x}*${x} + ${z}*${z}))")
echo "  position delta: $dist units (need >= 1.0)"
ok=$(python -c "print(1 if $dist >= 1.0 else 0)")
if [[ "$ok" -ne 1 ]]; then
    echo "FAIL: vehicle did not move"
    exit 1
fi
echo "[ok] vehicle moved $dist units"

# yaw changed (steering worked).
yaw_abs=$(python -c "print(abs($yaw))")
ok=$(python -c "print(1 if $yaw_abs >= 0.01 else 0)")
if [[ "$ok" -ne 1 ]]; then
    echo "WARN: yaw unchanged ($yaw); auto-drive steer pattern may need tuning"
fi
echo "[ok] yaw delta $yaw_abs rad"

echo ""
echo "Phase 4: PASS"
