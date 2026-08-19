#!/usr/bin/env python3
"""Verify Dreamland water/terrain occlusion from renderer query traces."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


PREFIX = "[DreamlandWaterOcclusion] "


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument(
        "--expect",
        choices=("protected", "unprotected"),
        required=True,
        help=(
            "protected requires zero leaked samples with depth enabled; "
            "unprotected requires the negative control to expose leaks"
        ),
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--allow-zero-visible",
        action="store_true",
        help=(
            "allow a deliberately positioned, fully terrain-covered water "
            "sample; the default route still requires visible water"
        ),
    )
    return parser.parse_args()


def load_summary(path: Path) -> dict[str, object]:
    summaries: list[dict[str, object]] = []
    intervals: list[dict[str, object]] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        marker = line.find(PREFIX)
        if marker < 0:
            continue
        payload = json.loads(line[marker + len(PREFIX) :])
        if payload.get("kind") == "summary":
            summaries.append(payload)
        elif payload.get("kind") == "interval":
            intervals.append(payload)
    if len(summaries) == 1:
        return summaries[0]
    if len(summaries) > 1:
        raise ValueError(
            f"expected exactly one water-occlusion summary, found {len(summaries)}"
        )
    if not intervals:
        raise ValueError("no water-occlusion summary or intervals were found")

    integer_fields = (
        "batches",
        "triangles",
        "candidate_samples",
        "visible_samples",
        "occluded_samples",
        "actual_samples",
        "leak_samples",
        "depth_enabled_batches",
        "depth_disabled_batches",
    )
    aggregate: dict[str, object] = {
        "kind": "aggregated_intervals",
        "interval_count": len(intervals),
    }
    for field in integer_fields:
        values = [record.get(field) for record in intervals]
        if not all(isinstance(value, int) for value in values):
            raise ValueError(f"interval field {field} is missing or invalid")
        aggregate[field] = sum(values)
    raster_values = {
        record.get("raster_depth_disabled") for record in intervals
    }
    if len(raster_values) != 1 or not raster_values <= {True, False}:
        raise ValueError("intervals disagree on raster-depth control state")
    aggregate["raster_depth_disabled"] = raster_values.pop()
    return aggregate


def verify(
    summary: dict[str, object],
    expected: str,
    allow_zero_visible: bool = False,
) -> list[str]:
    failures: list[str] = []

    def integer(name: str) -> int:
        value = summary.get(name)
        if not isinstance(value, int):
            failures.append(f"{name} is missing or not an integer")
            return 0
        return value

    batches = integer("batches")
    triangles = integer("triangles")
    candidates = integer("candidate_samples")
    visible = integer("visible_samples")
    occluded = integer("occluded_samples")
    actual = integer("actual_samples")
    leaks = integer("leak_samples")
    depth_enabled = integer("depth_enabled_batches")
    depth_disabled = integer("depth_disabled_batches")
    raster_disabled = summary.get("raster_depth_disabled")

    if batches <= 0:
        failures.append("no source-identified Dreamland water batches were traced")
    if triangles <= 0:
        failures.append("no source-identified Dreamland water triangles were traced")
    if candidates <= 0:
        failures.append("water produced no candidate raster samples")
    if visible <= 0 and not allow_zero_visible:
        failures.append("no water samples were visible; a hidden surface cannot pass")
    if occluded <= 0:
        failures.append(
            "scene produced no water samples behind nearer opaque geometry; "
            "the regression condition was not exercised"
        )
    if visible + occluded != candidates:
        failures.append(
            "depth partition is incomplete: "
            f"visible({visible}) + occluded({occluded}) != candidates({candidates})"
        )

    if expected == "protected":
        if raster_disabled is not False:
            failures.append("protected run did not report raster depth enabled")
        if depth_enabled != batches or depth_disabled != 0:
            failures.append(
                "not every Dreamland water batch used depth protection: "
                f"enabled={depth_enabled} disabled={depth_disabled} batches={batches}"
            )
        if actual != visible:
            failures.append(
                f"actual water samples ({actual}) differ from depth-visible samples ({visible})"
            )
        if leaks != 0:
            failures.append(f"{leaks} occluded water samples reached the actual draw")
    else:
        if raster_disabled is not True:
            failures.append("negative control did not report raster depth disabled")
        if depth_disabled != batches or depth_enabled != 0:
            failures.append(
                "negative control did not disable every water batch: "
                f"enabled={depth_enabled} disabled={depth_disabled} batches={batches}"
            )
        if actual != candidates:
            failures.append(
                f"unprotected actual samples ({actual}) differ from candidates ({candidates})"
            )
        if leaks != occluded or leaks <= 0:
            failures.append(
                "negative control did not expose the expected occluded water: "
                f"leaks={leaks} occluded={occluded}"
            )
    return failures


def main() -> int:
    args = parse_args()
    try:
        summary = load_summary(args.log)
        failures = verify(
            summary,
            args.expect,
            allow_zero_visible=args.allow_zero_visible,
        )
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        summary = {}
        failures = [str(exc)]

    report = {
        "schema": "v8.dreamland-water-occlusion.v1",
        "expectation": args.expect,
        "allow_zero_visible": args.allow_zero_visible,
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "trace_summary": summary,
        "source_log": str(args.log.resolve()),
    }
    encoded = json.dumps(report, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded + "\n", encoding="utf-8")
    print(encoded)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
