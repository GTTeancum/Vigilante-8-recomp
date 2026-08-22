#!/usr/bin/env python3
"""Verify native-conversion runtime logs without using raster captures."""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path


TAG_FIELDS_RE = re.compile(r"^\[(?P<tag>[^]]+)] (?P<fields>.*)$")
FIELD_RE = re.compile(r"([\w-]+)=([^\s]+)")
FATAL_MARKERS = (
    "unhandled exception",
    "fatal error",
    "unmapped call:",
    "accessviolationexception",
    "outofmemoryexception",
    "pc heap exhausted",
    "object scheduler exceeded",
    "entered drawsync wait without",
    "did not complete after",
    "rejected malformed shape stream",
)


def read_log(path: Path) -> str:
    data = path.read_bytes()
    if data.startswith((b"\xff\xfe", b"\xfe\xff")):
        return data.decode("utf-16")
    if data[:256].count(b"\x00") > 32:
        return data.decode("utf-16-le")
    return data.decode("utf-8", errors="replace")


def fields(value: str) -> dict[str, str]:
    return dict(FIELD_RE.findall(value))


def integer(sample: dict[str, str], name: str, default: int = 0) -> int:
    return int(sample.get(name, str(default)), 0)


def point(value: str) -> tuple[int, int]:
    x, y = value.split(",", 1)
    return int(x), int(y)


def extent(value: str) -> tuple[int, int]:
    w, h = value.split("x", 1)
    return int(w), int(h)


def contained(
    inner_xy: tuple[int, int],
    inner_wh: tuple[int, int],
    outer_xy: tuple[int, int],
    outer_wh: tuple[int, int],
) -> bool:
    ix, iy = inner_xy
    iw, ih = inner_wh
    ox, oy = outer_xy
    ow, oh = outer_wh
    return (
        ix >= ox
        and iy >= oy
        and ix + iw <= ox + ow
        and iy + ih <= oy + oh
    )


def load_samples(text: str) -> dict[str, list[dict[str, str]]]:
    samples: dict[str, list[dict[str, str]]] = {}
    for line in text.splitlines():
        match = TAG_FIELDS_RE.match(line)
        if not match:
            continue
        samples.setdefault(match.group("tag"), []).append(
            fields(match.group("fields"))
        )
    return samples


def hud_containment(
    packets: list[dict[str, str]],
) -> tuple[list[dict[str, object]], int]:
    matches: list[dict[str, object]] = []
    uncontained = 0
    for index, backing in enumerate(packets):
        if backing.get("health") != "1":
            continue
        if "xy" not in backing or "wh" not in backing:
            continue
        backing_xy = point(backing["xy"])
        backing_wh = extent(backing["wh"])
        candidate = None
        # V8:2 emits four 2x2 radar points between the health plate and its
        # fill. Restricting the search to the following eight HUD packets
        # identifies the native fill without hard-coding map coordinates,
        # texture pages, CLUTs, or player-screen offsets.
        for packet in packets[index + 1:index + 9]:
            if (
                packet.get("textured") == "1"
                and packet.get("health") == "0"
                and packet.get("radar") == "0"
                and packet.get("status-backing") == "0"
                and "xy" in packet
                and "wh" in packet
                and extent(packet["wh"])[0] <= backing_wh[0]
            ):
                candidate = packet
                break
        if candidate is None:
            continue
        candidate_xy = point(candidate["xy"])
        candidate_wh = extent(candidate["wh"])
        is_contained = contained(
            candidate_xy, candidate_wh, backing_xy, backing_wh
        )
        if not is_contained:
            uncontained += 1
        matches.append(
            {
                "plate_xy": backing_xy,
                "plate_wh": backing_wh,
                "fill_xy": candidate_xy,
                "fill_wh": candidate_wh,
                "contained": is_contained,
            }
        )
    return matches, uncontained


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_directory", type=Path)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--require-terrain-frames", type=int, default=120)
    parser.add_argument("--require-depth-windows", type=int, default=2)
    parser.add_argument("--require-fog-windows", type=int, default=2)
    parser.add_argument(
        "--require-converted-surface-windows", type=int, default=0
    )
    parser.add_argument("--require-glass", action="store_true")
    args = parser.parse_args()

    run_dir = args.run_directory.resolve()
    summary_path = run_dir / "summary.json"
    if not summary_path.is_file():
        raise SystemExit(f"missing soak summary: {summary_path}")
    log_paths = sorted(run_dir.glob("*.stderr.log"))
    if not log_paths:
        raise SystemExit(f"no stderr logs in {run_dir}")

    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    text = "\n".join(read_log(path) for path in log_paths)
    lower = text.lower()
    samples = load_samples(text)
    terrain = samples.get("TerrainFrame", [])
    depth = samples.get("Depth", [])
    fog = samples.get("EnhancedFogFrame", [])
    hud = samples.get("V82HudPacket", [])
    converted_surfaces = samples.get("ConvertedSurface", [])
    materials = Counter(
        sample.get("material", "unknown")
        for sample in samples.get("VehicleMaterialTriangle", [])
    )

    terrain_drops = [
        sample for sample in terrain
        if integer(sample, "terrain") <= 0
        or integer(sample, "poolDropped") != 0
        or "polyDrops[]" not in " ".join(
            f"{key}={value}" for key, value in sample.items()
        ) and sample.get("polyDrops", "[]") != "[]"
    ]
    active_depth = [sample for sample in depth if integer(sample, "transparent-tris") > 0]
    bad_depth = [
        sample for sample in active_depth
        if integer(sample, "painter-world") != 0
        or integer(sample, "world-transparent-depth")
        > integer(sample, "depth-tested")
        or integer(sample, "transparent-tris")
        != integer(sample, "world-transparent-depth")
        + integer(sample, "painter-screen")
    ]
    active_fog = [sample for sample in fog if integer(sample, "tick") > 0]
    bad_fog = [
        sample for sample in active_fog
        if integer(sample, "valid") != 1
        # Gameplay is presented at 60 Hz while new PS1 draw work commonly
        # arrives at 30 Hz. A repeat present legitimately retains the arena's
        # immediately preceding authored backdrop candidate; anything older
        # indicates stale atmosphere state crossing a real draw boundary.
        or integer(sample, "frame") - integer(sample, "selected-frame")
        not in (0, 1)
        or float(sample.get("span", "0")) <= 0.0
    ]
    hud_matches, uncontained_hud = hud_containment(hud)
    active_converted_surfaces = [
        sample for sample in converted_surfaces
        if integer(sample, "triangles") > 0
    ]
    visible_converted_surfaces = [
        sample for sample in active_converted_surfaces
        if integer(sample, "visible") > 0
        and float(sample.get("screen-area", "0")) > 0.0
        and float(sample.get("max-area", "0")) > 0.0
    ]
    bad_converted_surfaces = [
        sample for sample in active_converted_surfaces
        if integer(sample, "depth-tested") != integer(sample, "triangles")
    ]
    run_failures = [
        run for run in summary.get("runs", []) if not run.get("passed", False)
    ]
    fatal_hits = [marker for marker in FATAL_MARKERS if marker in lower]

    checks = {
        "soak_runs_passed": bool(summary.get("runs")) and not run_failures,
        "no_fatal_or_unmapped_diagnostics": not fatal_hits,
        "terrain_frames_present": len(terrain) >= args.require_terrain_frames,
        "terrain_has_no_empty_or_dropped_frames": bool(terrain) and not terrain_drops,
        "shared_depth_windows_present": len(active_depth) >= args.require_depth_windows,
        "all_world_transparency_uses_shared_depth": bool(active_depth) and not bad_depth,
        "fog_windows_present": len(active_fog) >= args.require_fog_windows,
        "fog_is_valid_and_current": bool(active_fog) and not bad_fog,
        "health_fill_identified": bool(hud_matches),
        "health_fill_contained_by_native_plate": bool(hud_matches) and uncontained_hud == 0,
    }
    if args.require_glass:
        checks["native_vehicle_glass_material_observed"] = (
            materials["OpaqueVehicleGlass"] > 0
            or materials["VehicleReflection"] > 0
        )
    if args.require_converted_surface_windows > 0:
        checks["converted_surface_windows_present"] = (
            len(active_converted_surfaces) >=
            args.require_converted_surface_windows
        )
        checks["converted_surface_is_visible_and_depth_tested"] = (
            bool(visible_converted_surfaces) and not bad_converted_surfaces
        )

    terrain_values = [integer(sample, "terrain") for sample in terrain]
    report = {
        "schema": "v82-native-conversion-diagnostics-v2",
        "passed": all(checks.values()),
        "checks": checks,
        "evidence": {
            "run_count": len(summary.get("runs", [])),
            "run_failures": run_failures,
            "fatal_hits": fatal_hits,
            "terrain": {
                "frames": len(terrain),
                "minimum": min(terrain_values) if terrain_values else None,
                "maximum": max(terrain_values) if terrain_values else None,
                "bad_frames": len(terrain_drops),
            },
            "depth": {
                "active_windows": len(active_depth),
                "bad_windows": len(bad_depth),
                "world_transparent_triangles": sum(
                    integer(sample, "world-transparent-depth")
                    for sample in active_depth
                ),
                "painter_world_triangles": sum(
                    integer(sample, "painter-world") for sample in active_depth
                ),
                "painter_screen_triangles": sum(
                    integer(sample, "painter-screen") for sample in active_depth
                ),
            },
            "fog": {
                "windows": len(active_fog),
                "bad_windows": len(bad_fog),
                "colors": sorted({sample.get("rgb") for sample in active_fog}),
                "span_range": [
                    min(float(sample["span"]) for sample in active_fog)
                    if active_fog else None,
                    max(float(sample["span"]) for sample in active_fog)
                    if active_fog else None,
                ],
                "candidate_age_range": [
                    min(
                        integer(sample, "frame") -
                        integer(sample, "selected-frame")
                        for sample in active_fog
                    ) if active_fog else None,
                    max(
                        integer(sample, "frame") -
                        integer(sample, "selected-frame")
                        for sample in active_fog
                    ) if active_fog else None,
                ],
            },
            "hud": {
                "packets": len(hud),
                "health_fill_matches": len(hud_matches),
                "uncontained": uncontained_hud,
                "unique_geometry": [
                    dict(zip(("plate_xy", "plate_wh", "fill_xy", "fill_wh"), key))
                    for key in sorted({
                        (
                            tuple(match["plate_xy"]),
                            tuple(match["plate_wh"]),
                            tuple(match["fill_xy"]),
                            tuple(match["fill_wh"]),
                        )
                        for match in hud_matches
                    })
                ],
            },
            "converted_surface": {
                "active_windows": len(active_converted_surfaces),
                "visible_windows": len(visible_converted_surfaces),
                "bad_windows": len(bad_converted_surfaces),
                "triangles": sum(
                    integer(sample, "triangles")
                    for sample in active_converted_surfaces
                ),
                "visible_triangles": sum(
                    integer(sample, "visible")
                    for sample in active_converted_surfaces
                ),
                "depth_tested_triangles": sum(
                    integer(sample, "depth-tested")
                    for sample in active_converted_surfaces
                ),
                "screen_area": sum(
                    float(sample.get("screen-area", "0"))
                    for sample in active_converted_surfaces
                ),
                "max_triangle_area": max(
                    (
                        float(sample.get("max-area", "0"))
                        for sample in active_converted_surfaces
                    ),
                    default=0.0,
                ),
            },
            "vehicle_material_triangles": dict(sorted(materials.items())),
        },
    }
    output = args.json or (run_dir / "conversion-diagnostics.json")
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    state = "PASS" if report["passed"] else "FAIL"
    print(
        f"[ConversionDiagnostics] {state} runs={len(summary.get('runs', []))} "
        f"terrain={len(terrain)} depth={len(active_depth)} fog={len(active_fog)} "
        f"hud-fills={len(hud_matches)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"  report={output}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
