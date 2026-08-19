#!/usr/bin/env python3
"""Verify source RSP route colours against the converted runtime trace."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import re


RAMP_RE = re.compile(
    r"\[V8N64RouteColorRamp\] low=(\d+),(\d+),(\d+) "
    r"high=(\d+),(\d+),(\d+) source=COLS"
)
VERTEX_RE = re.compile(
    r"\[V8N64RouteVertexColor\].*?"
    r"encoded=(\d+),(\d+),(\d+) index=(\d+) substep=(\d+) "
    r"mapped=(\d+),(\d+),(\d+)"
)
REJECT_RE = re.compile(r"\[V8N64RouteVertexColorReject\]")
SUMMARY_RE = re.compile(
    r"\[V8N64RouteColorSummary\] tick=(\d+) mapped=(\d+) rejected=(\d+)"
)


def trunc_div(numerator: int, denominator: int) -> int:
    magnitude = abs(numerator) // abs(denominator)
    return -magnitude if (numerator < 0) != (denominator < 0) else magnitude


def ramp(low: tuple[int, ...], high: tuple[int, ...]) -> list[tuple[int, ...]]:
    return [
        tuple(
            low[channel]
            + trunc_div((high[channel] - low[channel]) * index, 31)
            for channel in range(3)
        )
        for index in range(32)
    ]


def shade_rgb(
    low: tuple[int, ...], high: tuple[int, ...], shade: int
) -> tuple[int, ...]:
    return tuple(
        low[channel]
        + trunc_div((high[channel] - low[channel]) * shade, 124)
        for channel in range(3)
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-proof", type=Path, required=True)
    parser.add_argument("--port-log", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    source = json.loads(args.source_proof.read_text(encoding="utf-8"))
    log = args.port_log.read_text(encoding="utf-8", errors="replace")
    failures: list[str] = []

    low = tuple(source["cols"]["route_ramp_low_rgb"])
    high = tuple(source["cols"]["route_ramp_high_rgb"])
    expected = ramp(low, high)
    if source.get("status") != "PASS":
        failures.append("source RSP route proof did not pass")
    if [list(value) for value in expected] != source["cols"]["ramp_rgb"]:
        failures.append("source proof ramp differs from signed /31 interpolation")

    ramp_matches = [
        tuple(int(group) for group in match.groups())
        for match in RAMP_RE.finditer(log)
    ]
    expected_endpoints = low + high
    if not ramp_matches:
        failures.append("port did not log a COLS route ramp")
    elif any(value != expected_endpoints for value in ramp_matches):
        failures.append("port COLS endpoints differ from the source arena")

    observed_indices: Counter[int] = Counter()
    observed_shades: Counter[int] = Counter()
    encoded_values: Counter[tuple[int, int, int]] = Counter()
    mapped_values: Counter[tuple[int, int, int]] = Counter()
    vertex_records = 0
    for match in VERTEX_RE.finditer(log):
        values = tuple(int(group) for group in match.groups())
        encoded = values[:3]
        index = values[3]
        substep = values[4]
        mapped = values[5:]
        shade = encoded[0]
        vertex_records += 1
        observed_indices[index] += 1
        observed_shades[shade] += 1
        encoded_values[encoded] += 1
        mapped_values[mapped] += 1
        if encoded != (shade,) * 3:
            failures.append(
                f"encoded route colour {encoded} is not grayscale"
            )
        if index != shade >> 2 or substep != (shade & 3):
            failures.append(
                f"shade {shade} logged inconsistent index/substep {index}/{substep}"
            )
        if not 0 <= shade <= 124:
            failures.append(f"route shade {shade} is outside 0..124")
        elif not 0 <= index < 32:
            failures.append(f"route colour index {index} is outside 0..31")
        elif mapped != shade_rgb(low, high, shade):
            failures.append(
                f"shade {shade} mapped to {mapped}, expected "
                f"{shade_rgb(low, high, shade)}"
            )
    if vertex_records == 0:
        failures.append("port logged no decoded route vertices")
    if 14 not in observed_indices:
        failures.append("port trace omitted Dreamland's dominant route index 14")

    rejects = len(REJECT_RE.findall(log))
    if rejects:
        failures.append(f"port rejected {rejects} N64 route vertex colours")
    summaries = [
        tuple(int(group) for group in match.groups())
        for match in SUMMARY_RE.finditer(log)
    ]
    if any(rejected != 0 for _tick, _mapped, rejected in summaries):
        failures.append("a runtime route-colour summary reports rejected vertices")

    source_indices = {
        int(item["index"]) for item in source["gradient_indices_present"]
    }
    overlap = source_indices & set(observed_indices)
    if len(overlap) < 8:
        failures.append(
            f"only {len(overlap)} source gradient indices appeared in port trace"
        )

    fatal_terms = (
        "shader compile failed",
        "shader link failed",
        "unhandled exception",
        "vigilante 8 fatal error",
    )
    fatal_hits = [term for term in fatal_terms if term in log.lower()]
    if fatal_hits:
        failures.append(f"fatal runtime markers: {fatal_hits}")

    report = {
        "schema": "v8.dreamland-route-color-parity.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "source_proof": str(args.source_proof.resolve()),
        "port_log": str(args.port_log.resolve()),
        "source_ramp_low_rgb": list(low),
        "source_ramp_high_rgb": list(high),
        "source_gradient_indices": sorted(source_indices),
        "port_vertex_records": vertex_records,
        "port_shades": [
            {"shade": shade, "vertices": count,
             "mapped_rgb": list(shade_rgb(low, high, shade))}
            for shade, count in sorted(observed_shades.items())
        ],
        "port_gradient_indices": [
            {"index": index, "vertices": count, "mapped_rgb": list(expected[index])}
            for index, count in sorted(observed_indices.items())
        ],
        "source_port_index_overlap": sorted(overlap),
        "runtime_ramp_records": [list(value) for value in ramp_matches],
        "runtime_summary_records": [
            {"tick": tick, "mapped": mapped, "rejected": rejected}
            for tick, mapped, rejected in summaries
        ],
        "runtime_reject_records": rejects,
        "encoded_colours": [
            {"rgb": list(rgb), "vertices": count}
            for rgb, count in encoded_values.most_common()
        ],
        "mapped_colours": [
            {"rgb": list(rgb), "vertices": count}
            for rgb, count in mapped_values.most_common()
        ],
        "fatal_runtime_markers": fatal_hits,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "status": report["status"],
        "source_indices": report["source_gradient_indices"],
        "port_indices": [item["index"] for item in report["port_gradient_indices"]],
        "vertex_records": vertex_records,
        "reject_records": rejects,
        "failures": failures,
    }, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
