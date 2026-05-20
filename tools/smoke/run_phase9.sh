#!/usr/bin/env bash
# Phase 9 -- the ACCEPTANCE TEST for "running game" deliverable.
#
# 60 seconds @60Hz = 3600 frames of autoplay. Must:
#   1. Run cleanly (exit 0)
#   2. Vehicle moves substantially (player drives around)
#   3. AI is reachable / hit at least once (combat occurs)
#   4. At least some destructibles broken (props interact)
#   5. Audio mixer emits non-silent output throughout
#   6. Screenshot is non-uniform (visual scene present)
#
# This is the "running game" milestone. Pass = the user can run the
# binary, see a window, watch a vehicle drive on terrain, see things
# break and AI move around, and hear sounds. That's the deal.

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 9 -- ACCEPTANCE TEST ==="
mkdir -p tools/smoke/captures

echo "  building..."
cmake --build build --config Debug > /tmp/p9_build.log 2>&1 || {
    echo "FAIL: build"; tail -15 /tmp/p9_build.log; exit 1;
}
echo "[ok] build clean"

SHOT=tools/smoke/captures/phase9.ppm
WAV=tools/smoke/captures/phase9.wav
rm -f "$SHOT" "$WAV"

echo "  running 60s autoplay (3600 frames) -- a window will open..."
# NOT --headless: acceptance test needs the GL backbuffer for the
# screenshot artifact. The window opens and closes when the cap hits.
timeout 90 ./build/Debug/v8.exe \
    --frames 3600 --auto-drive 500 --auto-fire 10 \
    --screenshot "$SHOT" --audio-capture "$WAV" \
    > /tmp/p9_run.log 2>&1
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: v8 exited $rc (expected 0; was the run too long?)"
    tail -10 /tmp/p9_run.log
    exit 1
fi
echo "[ok] v8 ran 3600 frames, exit 0"

# 1. Screenshot non-uniform.
python tools/smoke/check_screenshot.py "$SHOT" || { echo "FAIL: screenshot uniform"; exit 1; }

# 2. Audio non-silent.
python tools/smoke/check_audio.py "$WAV" || { echo "FAIL: audio silent"; exit 1; }

# 3. Vehicle moved.
pos_line=$(grep "vehicle pos =" /tmp/p9_run.log | tail -1)
read x z <<<$(python -c "
import re
m = re.search(r'\(([-\d.]+), [-\d.]+, ([-\d.]+)\)', '''$pos_line''')
print(m.group(1), m.group(2)) if m else print('0 0')
")
dist=$(python -c "import math; print(math.sqrt($x*$x + $z*$z))")
ok=$(python -c "print(1 if $dist >= 5.0 else 0)")
[[ "$ok" -eq 1 ]] || { echo "FAIL: vehicle barely moved ($dist units)"; exit 1; }
echo "[ok] vehicle moved $dist units (>= 5)"

# 4. AI hit at least once.
if ! grep -q "AI hit!" /tmp/p9_run.log; then
    echo "FAIL: AI was never hit"
    exit 1
fi
n_ai_hits=$(grep -c "AI hit!" /tmp/p9_run.log)
echo "[ok] AI hit $n_ai_hits times"

# 5. At least 3 destructibles broken.
n_broken=$(grep "destructibles broken =" /tmp/p9_run.log | tail -1 | grep -oE "[0-9]+" | tail -1)
[[ -n "$n_broken" && "$n_broken" -ge 3 ]] || {
    echo "FAIL: only ${n_broken:-0} destructibles broken (need >= 3)"; exit 1;
}
echo "[ok] $n_broken destructibles broken"

# 6. Projectiles spawned plenty.
n_proj=$(grep "projectiles spawned" /tmp/p9_run.log | tail -1 | grep -oE "[0-9]+" | tail -1)
[[ -n "$n_proj" && "$n_proj" -ge 100 ]] || {
    echo "FAIL: only ${n_proj:-0} projectiles spawned (need >= 100)"; exit 1;
}
echo "[ok] $n_proj projectiles spawned"

echo ""
echo "============================================================"
echo "Phase 9: PASS -- RUNNING GAME DELIVERED"
echo "============================================================"
echo "  60 seconds of autoplay, all checks green."
echo "  Vehicle drove $dist units. AI hit $n_ai_hits times."
echo "  $n_broken props broken. $n_proj projectiles spawned."
echo "  Screenshot has visible terrain/vehicles/props."
echo "  Audio output emitted sounds for the duration."
echo "============================================================"
