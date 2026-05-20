#!/usr/bin/env bash
# Phase 5 smoke test. Exit 0 = pass.
#
# Checks:
#   1. build clean
#   2. v8 --frames 300 --auto-drive 250 --auto-fire 20 exits 0
#   3. log records >= 5 projectiles spawned
#   4. screenshot remains non-uniform
#   5. vehicle still moves (Phase 4 regression check)

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 5 Smoke Test ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p5_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p5_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase5.ppm
rm -f "$SHOT"
timeout 30 ./build/Debug/v8.exe \
    --frames 300 --auto-drive 250 --auto-fire 20 \
    --screenshot "$SHOT" \
    > /tmp/p5_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc"
    tail -10 /tmp/p5_run.log
    exit 1
fi
echo "[ok] v8 ran 300 frames with auto-fire, exit 0"

if [[ ! -f "$SHOT" ]]; then
    echo "FAIL: screenshot not produced"
    exit 1
fi
python tools/smoke/check_screenshot.py "$SHOT" || {
    echo "FAIL: screenshot uniform color"; exit 1;
}

# Projectile count.
proj_line=$(grep "projectiles spawned" /tmp/p5_run.log | tail -1)
n_proj=$(echo "$proj_line" | grep -oE "[0-9]+" | tail -1)
if [[ -z "$n_proj" ]]; then
    echo "FAIL: no projectile count in log"
    exit 1
fi
echo "  $proj_line"
if [[ "$n_proj" -lt 5 ]]; then
    echo "FAIL: only $n_proj projectiles spawned (need >= 5)"
    exit 1
fi
echo "[ok] $n_proj projectiles spawned"

# Vehicle still moves.
pos_line=$(grep "vehicle pos =" /tmp/p5_run.log | tail -1)
read x z <<<$(python -c "
import re
m = re.search(r'\(([-\d.]+), [-\d.]+, ([-\d.]+)\)', '''$pos_line''')
print(m.group(1), m.group(2)) if m else print('0 0')
")
dist=$(python -c "import math; print(math.sqrt($x*$x + $z*$z))")
ok=$(python -c "print(1 if $dist >= 1.0 else 0)")
if [[ "$ok" -ne 1 ]]; then
    echo "FAIL: vehicle did not move (regression from Phase 4)"
    exit 1
fi
echo "[ok] vehicle moved $dist units"

echo ""
echo "Phase 5: PASS"
