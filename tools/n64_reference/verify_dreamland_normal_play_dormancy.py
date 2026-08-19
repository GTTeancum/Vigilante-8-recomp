#!/usr/bin/env python3
"""Verify that the Dreamland proof seed hook is dormant in normal play."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re


ROUTE_FIELDS = (
    "terrain-route",
    "route-opaque",
    "route-transparent",
    "route-depth-write",
    "route-depth-test",
    "route-depth-compare-write",
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def key_values(line: str) -> dict[str, int]:
    return {
        key: int(value)
        for key, value in re.findall(r"([a-z][a-z-]+)=(\d+)", line)
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--summary", type=Path, required=True)
    parser.add_argument("--stdout", type=Path, required=True)
    parser.add_argument("--stderr", type=Path, required=True)
    parser.add_argument("--deployed-exe", type=Path, required=True)
    parser.add_argument("--tested-exe", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--minimum-tick", type=int, default=360)
    args = parser.parse_args()

    failures: list[str] = []
    summary = json.loads(args.summary.read_text(encoding="utf-8"))
    if not isinstance(summary, list) or len(summary) != 1:
        failures.append("normal-play summary must contain exactly one run")
        run: dict[str, object] = {}
    else:
        run = summary[0]
    if not run.get("passed"):
        failures.append("normal-play Dreamland run did not pass")
    if not run.get("clean_match_exit"):
        failures.append("normal-play Dreamland run did not exit cleanly")
    if int(run.get("last_gameplay_tick", 0)) < args.minimum_tick:
        failures.append("normal-play Dreamland run ended before the minimum tick")

    stdout = args.stdout.read_text(encoding="utf-8", errors="replace")
    stderr = args.stderr.read_text(encoding="utf-8", errors="replace")
    deterministic_seed_log_lines = sum(
        "[V8DeterministicRng]" in line
        for line in (stdout + "\n" + stderr).splitlines()
    )
    if deterministic_seed_log_lines:
        failures.append("proof-only deterministic seed hook activated")

    route_intervals: list[dict[str, int]] = []
    for line in stderr.splitlines():
        if "[EnhancedRenderer]" not in line:
            continue
        values = key_values(line)
        if values.get("terrain-route", 0) > 0:
            route_intervals.append(
                {field: values.get(field, 0) for field in ROUTE_FIELDS}
            )
    if not route_intervals:
        failures.append("normal-play trace contains no route-bearing intervals")
    for index, interval in enumerate(route_intervals):
        route = interval["terrain-route"]
        if not (
            interval["route-opaque"] == route
            and interval["route-transparent"] == 0
            and interval["route-depth-write"] == route
            and interval["route-depth-test"] == route
            and interval["route-depth-compare-write"] == route
        ):
            failures.append(
                f"normal-play route interval {index} violates depth parity"
            )

    deployed_hash = sha256(args.deployed_exe)
    tested_hash = sha256(args.tested_exe)
    if deployed_hash != tested_hash:
        failures.append("deployed executable differs from the tested candidate")

    report = {
        "schema": "v8.dreamland-normal-play-dormancy.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "deterministic_seed_log_lines": deterministic_seed_log_lines,
        "last_gameplay_tick": int(run.get("last_gameplay_tick", 0)),
        "clean_match_exit": bool(run.get("clean_match_exit")),
        "route_intervals": route_intervals,
        "route_triangles": sum(
            interval["terrain-route"] for interval in route_intervals
        ),
        "deployed_exe_sha256": deployed_hash,
        "tested_exe_sha256": tested_hash,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
