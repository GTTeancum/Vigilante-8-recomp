#!/usr/bin/env bash
# Phase 6 smoke test. Exit 0 = pass.
#
# Checks:
#   1. build clean
#   2. v8 --frames 600 --auto-fire 12 exits 0 (player stationary,
#      AI drives toward player, projectiles fly forward)
#   3. AI position changed from spawn (moved toward player)
#   4. AI got hit at least once (HP < starting value, or "AI hit!" log)
#   5. Screenshot non-uniform

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 6 Smoke Test ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p6_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p6_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase6.ppm
rm -f "$SHOT"
timeout 30 ./build/Debug/v8.exe \
    --frames 600 --auto-fire 12 \
    --screenshot "$SHOT" \
    > /tmp/p6_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc"
    tail -10 /tmp/p6_run.log
    exit 1
fi
echo "[ok] v8 ran 600 frames, exit 0"

python tools/smoke/check_screenshot.py "$SHOT" || {
    echo "FAIL: screenshot uniform"; exit 1;
}

# AI moved.
ai_line=$(grep "AI pos =" /tmp/p6_run.log | tail -1)
read aix aiz <<<$(python -c "
import re
m = re.search(r'\(([-\d.]+), ([-\d.]+)\)', '''$ai_line''')
print(m.group(1), m.group(2)) if m else print('12 12')
")
# AI starts at (12, 12); moved if distance from start > 1
ai_delta=$(python -c "import math; print(math.sqrt(($aix-12)**2 + ($aiz-12)**2))")
ok=$(python -c "print(1 if $ai_delta >= 1.0 else 0)")
if [[ "$ok" -ne 1 ]]; then
    echo "FAIL: AI did not move from spawn (delta=$ai_delta)"
    exit 1
fi
echo "[ok] AI moved $ai_delta units from spawn"

# AI got hit (HP decremented or 'AI hit!' line).
if ! grep -q "AI hit!" /tmp/p6_run.log; then
    echo "FAIL: AI was never hit by player projectiles"
    exit 1
fi
n_hits=$(grep -c "AI hit!" /tmp/p6_run.log)
echo "[ok] AI hit $n_hits times"

echo ""
echo "Phase 6: PASS"
