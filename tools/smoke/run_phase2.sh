#!/usr/bin/env bash
# Phase 2 smoke test. Exit 0 = pass.
#
# Checks:
#   1. cmake --build succeeds
#   2. v8 --frames 300 --headless exits 0 (no segfault, no timeout)
#   3. Output records Quest.bin loaded
#   4. Output records the frame cap was reached (not an early exit)

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 2 Smoke Test ==="

# Build.
echo "  building..."
cmake --build build --config Debug > /tmp/p2_build.log 2>&1 || {
    echo "FAIL: build error"; tail -15 /tmp/p2_build.log; exit 1;
}
echo "[ok] build clean"

# Run 300 headless frames; timeout 20s safety net.
timeout 20 ./build/Debug/v8.exe --frames 300 --headless > /tmp/p2_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc (expected 0)"
    tail -10 /tmp/p2_run.log
    exit 1
fi
echo "[ok] v8 ran 300 frames headless, exit 0"

# Quest.bin loaded.
if ! grep -q "Asset_LoadFile(Quest.bin)" /tmp/p2_run.log; then
    echo "FAIL: Quest.bin was not loaded"
    exit 1
fi
echo "[ok] Quest.bin loaded"

# Frame cap reached (not bailed early).
if ! grep -q "frame cap 300" /tmp/p2_run.log; then
    echo "FAIL: frame cap not reached -- ran fewer than 300 frames"
    tail -5 /tmp/p2_run.log
    exit 1
fi
echo "[ok] frame cap 300 reached"

echo ""
echo "Phase 2: PASS"
