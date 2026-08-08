#!/usr/bin/env python3
"""Turn V8:2 runtime instrumentation into explicit regression pass/fail checks."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


BUILD_RE = re.compile(
    r"\[V82Vehicles\] built (?P<id>\S+) selector=0x(?P<body>[0-9A-F]+) "
    r"wheels=0x(?P<wheels>[0-9A-F]+) "
    r"selector_live_pointers=(?P<live>\d+) "
    r"pc_allocations=(?P<alloc>\d+)"
)
RELEASE_RE = re.compile(
    r"\[V82SelectorResources\] released=(?P<id>\S+) reason=(?P<reason>\S+) "
    r"pc_allocations=(?P<alloc>\d+) vram_reservations=(?P<vram>\d+) "
    r"\s*selector_live_pointers=(?P<live>\d+)"
)
PERF_RE = re.compile(r"\[EnhancedPerformance\] (?P<fields>[^\r\n]+)")
HUD_RE = re.compile(r"\[V82HudPacket\] (?P<fields>[^\r\n]+)")


def fields(value: str) -> dict[str, str]:
    return dict(re.findall(r"([\w-]+)=([^\s]+)", value))


def point(value: str) -> tuple[int, int]:
    x, y = value.split(",", 1)
    return int(x), int(y)


def read_log(path: Path) -> str:
    data = path.read_bytes()
    if data.startswith((b"\xff\xfe", b"\xfe\xff")):
        return data.decode("utf-16")
    if data[:256].count(b"\x00") > 32:
        return data.decode("utf-16-le")
    return data.decode("utf-8", errors="replace")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--require-selector-builds", type=int, default=2)
    parser.add_argument("--require-performance-windows", type=int, default=2)
    parser.add_argument("--max-frame-mean-ms", type=float, default=20.0)
    parser.add_argument(
        "--scope",
        choices=("all", "selector", "gameplay"),
        default="all",
    )
    args = parser.parse_args()
    text = read_log(args.log)

    builds = [match.groupdict() for match in BUILD_RE.finditer(text)]
    releases = [match.groupdict() for match in RELEASE_RE.finditer(text)]
    performance = [
        fields(match.group("fields")) for match in PERF_RE.finditer(text)
    ]
    hud = [fields(match.group("fields")) for match in HUD_RE.finditer(text)]
    released_by_id: dict[str, int] = {}
    for release in releases:
        released_by_id[release["id"]] = (
            released_by_id.get(release["id"], 0) + 1
        )
    unmatched = [
        build["id"]
        for build in builds
        if released_by_id.get(build["id"], 0) == 0
    ]

    # Native menus and FMV deliberately render in 4:3 and can be stream-paced.
    # Gameplay windows are unambiguous because only they submit widened GTE
    # projection vertices. Apply performance thresholds only to those windows.
    gameplay_performance = [
        sample
        for sample in performance
        if int(sample.get("wide-projection-vertices", "0")) > 0
    ]
    steady = (
        gameplay_performance[1:]
        if len(gameplay_performance) > 1
        else gameplay_performance
    )
    status_packets = [
        packet for packet in hud if packet.get("status-backing") == "1"
    ]
    right_hud_contents = [
        packet
        for packet in hud
        if packet.get("status-backing") != "1"
        and packet.get("textured") == "1"
        and "xy" in packet
        and 74 <= point(packet["xy"])[0] < 170
        and (
            12 <= point(packet["xy"])[1] < 60
            or 252 <= point(packet["xy"])[1] < 300
        )
    ]
    common_checks = {
        "no_fatal": not any(
            marker in text
            for marker in (
                "[Fatal]",
                "Unhandled exception",
                "Application Error",
                "unknown software exception",
            )
        ),
    }
    selector_checks = {
        "selector_exercised":
            len(builds) >= args.require_selector_builds,
        "selector_builds_released": not unmatched,
        "selector_native_allocations_drained":
            bool(releases)
            and all(int(release["live"]) == 0 for release in releases),
        "selector_vram_drained":
            bool(releases)
            and all(int(release["vram"]) == 0 for release in releases),
    }
    gameplay_checks = {
        "performance_sampled":
            len(gameplay_performance) >=
            args.require_performance_windows,
        "steady_presentation_extent_stable":
            bool(steady)
            and all(
                int(sample["presentation-extent-switches"]) == 0
                for sample in steady
            ),
        "steady_no_present_reallocation":
            bool(steady)
            and all(
                int(sample["present-reallocations"]) == 0
                for sample in steady
            ),
        "steady_no_vram_fallback":
            bool(steady)
            and all(
                int(sample["vram-fallback-presents"]) == 0
                for sample in steady
            ),
        "steady_frame_time":
            bool(steady)
            and all(
                float(sample["frame-mean-ms"])
                <= args.max_frame_mean_ms
                for sample in steady
            ),
        "widescreen_projection_active":
            bool(steady)
            and any(
                int(sample["wide-expanded-vertices"]) > 0
                for sample in steady
            ),
        "hud_status_backing_exact":
            bool(status_packets)
            and all(
                packet.get("uv") == "104,32"
                and packet.get("wh") == "84x34"
                for packet in status_packets
            ),
        "hud_live_content_preserved": bool(right_hud_contents),
    }
    checks = dict(common_checks)
    if args.scope in ("all", "selector"):
        checks.update(selector_checks)
    if args.scope in ("all", "gameplay"):
        checks.update(gameplay_checks)
    report = {
        "passed": all(checks.values()),
        "checks": checks,
        "selector": {
            "builds": len(builds),
            "releases": len(releases),
            "unmatched": unmatched,
            "terminal_pc_allocations":
                int(releases[-1]["alloc"]) if releases else None,
            "terminal_vram_reservations":
                int(releases[-1]["vram"]) if releases else None,
            "terminal_live_pointers":
                int(releases[-1]["live"]) if releases else None,
        },
        "performance": {
            "windows": len(performance),
            "gameplay_windows": len(gameplay_performance),
            "steady_windows": len(steady),
            "samples": performance,
        },
        "hud": {
            "packets": len(hud),
            "status_backings": len(status_packets),
            "right_hud_contents": len(right_hud_contents),
        },
    }
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8"
        )
    state = "PASS" if report["passed"] else "FAIL"
    print(
        f"[IntegratedDiagnostics] {state} "
        f"selector={len(builds)}/{len(releases)} "
        f"perf-windows={len(performance)} hud-packets={len(hud)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
