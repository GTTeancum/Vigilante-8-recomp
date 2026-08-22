#!/usr/bin/env python3
"""Verify an imported-arena-to-retail selector round trip from text logs."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


FATAL_MARKERS = (
    "unhandled exception", "fatal error", "unmapped call:",
    "accessviolationexception", "outofmemoryexception",
)
MEDIA = {".ppm", ".png", ".jpg", ".jpeg", ".webp", ".bmp", ".gif", ".mp4"}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_directory", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--imported-slot", type=int, default=18)
    parser.add_argument("--retail-overlay", default="LEVELS_ROUTE66")
    args = parser.parse_args()

    run_dir = args.run_directory.resolve()
    summary = json.loads((run_dir / "summary.json").read_text(encoding="utf-8"))
    runs = summary.get("runs", [])
    if len(runs) != 1:
        raise ValueError(f"expected one run, got {len(runs)}")
    run = runs[0]
    stdout = Path(run["stdout_log"]).read_text(encoding="utf-8", errors="replace")
    stderr = Path(run["stderr_log"]).read_text(encoding="utf-8", errors="replace")
    fixture = next(run_dir.glob("*.input.txt")).read_text(encoding="utf-8")
    combined_lower = (stdout + "\n" + stderr).lower()
    right_count = sum(line.endswith("=RIGHT") for line in fixture.splitlines())
    left_count = sum(line.endswith("=LEFT") for line in fixture.splitlines())
    fatal_hits = [marker for marker in FATAL_MARKERS if marker in combined_lower]
    retained_media = [
        str(path) for path in run_dir.rglob("*")
        if path.is_file() and path.suffix.lower() in MEDIA
    ]
    checks = {
        "run_passed": run.get("passed") is True,
        "visited_imported_slot": (
            right_count == args.imported_slot
            and (
                f"[V82ArenaSelectorRecord] slot={args.imported_slot} " in stdout
                and 'title="Super Dreamland 64" ' in stdout
                and 'subtitle-text="Super Dreamland 64"' in stdout
            )
        ),
        "returned_to_retail_zero": left_count == args.imported_slot,
        "long_subtitle_backing_expanded": (
            "[V82ArenaTitleLayout] expanded native subtitle backing" in stdout
            and "width=192->272" in stdout
        ),
        "long_subtitle_backing_restored": (
            "[V82ArenaTitleLayout] restored native subtitle backing" in stdout
            and "width=192" in stdout
        ),
        "native_extended_table_loaded": (
            f"resources={args.imported_slot + 2} previews={args.imported_slot + 1} "
            f"background-index={args.imported_slot + 1}" in stdout
        ),
        "final_location_is_retail_zero": "[V82Arena] location=retail.0" in stdout,
        "retail_overlay_launched_once": (
            stdout.count(f"loaded relocated overlay: {args.retail_overlay}") == 1
            and run.get("actual_overlay") == args.retail_overlay
        ),
        "imported_gameplay_overlay_not_loaded": (
            stdout.count("loaded relocated overlay: LEVELS_N64_DREAMLND") == 0
        ),
        "no_fatal_markers": not fatal_hits,
        "no_raster_artifacts": not retained_media,
    }
    report = {
        "schema": "v82-arena-selector-roundtrip-v1",
        "passed": all(checks.values()),
        "checks": checks,
        "evidence": {
            "rightInputs": right_count,
            "leftInputs": left_count,
            "actualOverlay": run.get("actual_overlay"),
            "gameplayFrames": run.get("last_frame"),
            "fatalMarkers": fatal_hits,
            "retainedMedia": retained_media,
        },
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    status = "PASS" if report["passed"] else "FAIL"
    print(
        f"[ArenaSelectorRoundtrip] {status} right={right_count} left={left_count} "
        f"overlay={run.get('actual_overlay')} media={len(retained_media)}"
    )
    print(f"  report={args.output}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
