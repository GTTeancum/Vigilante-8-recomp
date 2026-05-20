#!/usr/bin/env bash
# Phase 7 smoke test. Exit 0 = pass.

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 7 Smoke Test ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p7_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p7_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase7.ppm
rm -f "$SHOT"
timeout 30 ./build/Debug/v8.exe \
    --frames 600 --auto-drive 500 --auto-fire 8 \
    --screenshot "$SHOT" \
    > /tmp/p7_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc"
    tail -10 /tmp/p7_run.log
    exit 1
fi
echo "[ok] v8 ran 600 frames, exit 0"

python tools/smoke/check_screenshot.py "$SHOT" || {
    echo "FAIL: screenshot uniform"; exit 1;
}

# Destructibles broken.
n_broken=$(grep "destructibles broken =" /tmp/p7_run.log | tail -1 | grep -oE "[0-9]+" | tail -1)
if [[ -z "$n_broken" ]] || [[ "$n_broken" -lt 1 ]]; then
    echo "FAIL: 0 destructibles broken"
    exit 1
fi
echo "[ok] $n_broken destructibles broken"

echo ""
echo "Phase 7: PASS"
