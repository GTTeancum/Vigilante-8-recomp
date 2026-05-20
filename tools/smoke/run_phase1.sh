#!/usr/bin/env bash
# Phase 1 smoke test. Exit 0 = pass.
#
# Checks:
#   1. cmake --build succeeds (compile + link clean)
#   2. v8.exe --selftest exits 0
#   3. selftest stdout reports "PASS"
#   4. v8.exe (no args) exits 0 and prints the Phase 0 banner

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 1 Smoke Test ==="

# 1. Build clean
echo "  building..."
cmake --build build --config Debug > /tmp/p1_build.log 2>&1
n_errors=$(grep -c "error C[0-9]" /tmp/p1_build.log || true)
if [[ "$n_errors" -gt 0 ]]; then
    echo "FAIL: $n_errors compile errors"
    grep "error C[0-9]" /tmp/p1_build.log | head -5
    exit 1
fi
if [[ ! -f build/Debug/v8.exe ]]; then
    echo "FAIL: build/Debug/v8.exe missing"
    exit 1
fi
echo "[ok] cmake build clean ($(stat -c %s build/Debug/v8.exe 2>/dev/null || stat -f %z build/Debug/v8.exe) bytes)"

# 2 + 3. --selftest
out=$(./build/Debug/v8.exe --selftest 2>&1)
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: --selftest exited $rc"
    echo "$out"
    exit 1
fi
if ! echo "$out" | grep -q "PASS"; then
    echo "FAIL: --selftest did not report PASS"
    echo "$out"
    exit 1
fi
echo "[ok] --selftest exit 0, output PASS"

# 4. default invocation
out=$(./build/Debug/v8.exe 2>&1)
rc=$?
if [[ "$rc" -ne 0 ]]; then
    echo "FAIL: default exit $rc"
    exit 1
fi
echo "[ok] default invocation exit 0"

echo ""
echo "Phase 1: PASS"
