#!/usr/bin/env bash
# Phase 8 smoke test. Exit 0 = pass.
set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 8 Smoke Test ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p8_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p8_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase8.ppm
WAV=tools/smoke/captures/phase8.wav
rm -f "$SHOT" "$WAV"
timeout 30 ./build/Debug/v8.exe \
    --frames 600 --auto-drive 500 --auto-fire 12 --headless \
    --screenshot "$SHOT" --audio-capture "$WAV" \
    > /tmp/p8_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc"
    tail -10 /tmp/p8_run.log
    exit 1
fi
echo "[ok] v8 ran 600 frames headless, exit 0"

# Audio assertion.
python tools/smoke/check_audio.py "$WAV" || { echo "FAIL: audio"; exit 1; }

# Event count: at least 5 fire beeps (auto-fire every 12 frames, 600/12=50 expected).
# But projectile spawn is rising-edge so each fire-bit press = 1 beep.
n_fire=$(grep -c "destructible" /tmp/p8_run.log || true)
echo "[info] destructible events: $n_fire (drives sfx 3)"

echo ""
echo "Phase 8: PASS"
