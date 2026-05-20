#!/usr/bin/env bash
# Phase 0 smoke test. Exit 0 = pass.
#
# Checks:
#   1. git log shows baseline + ghidra-ref commits
#   2. cmake -B build configures cleanly
#   3. third_party has headers
#   4. ≥1 cleaned C file has its #if 0 Ghidra ref block attached

set -e
cd "$(dirname "$0")/../.."

echo "=== Phase 0 Smoke Test ==="

# 1. git log
n_commits=$(git log --oneline 2>/dev/null | wc -l)
if [[ "$n_commits" -lt 2 ]]; then
    echo "FAIL: expected >=2 commits, got $n_commits"
    exit 1
fi
echo "[ok] git log shows $n_commits commits"

# 2. cmake configure
if [[ ! -d build ]]; then
    echo "  configuring..."
    cmake -B build -G "Visual Studio 17 2022" -A x64 > /dev/null
fi
if [[ ! -f build/CMakeCache.txt ]]; then
    echo "FAIL: CMakeCache.txt missing -- configure failed"
    exit 1
fi
echo "[ok] cmake configure succeeded"

# 3. third_party headers
n_headers=$(find third_party -name "*.h" -type f 2>/dev/null | wc -l)
if [[ "$n_headers" -lt 1 ]]; then
    echo "FAIL: third_party has no headers"
    exit 1
fi
echo "[ok] third_party has $n_headers headers"

# 4. ghidra ref blocks (recursive)
n_refs=$(find src -name "*.c" -exec grep -l "GHIDRA REF (audit ground truth" {} \; | wc -l)
if [[ "$n_refs" -lt 150 ]]; then
    echo "FAIL: only $n_refs files have ghidra ref blocks (expected >=150)"
    exit 1
fi
echo "[ok] $n_refs files have ghidra ref blocks attached"

echo ""
echo "Phase 0: PASS"
