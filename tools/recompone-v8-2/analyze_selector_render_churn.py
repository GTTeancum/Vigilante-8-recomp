#!/usr/bin/env python3
"""Audit repeated V8:2 selector preview construction from verbose text logs."""

from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re


FRAME = re.compile(
    r"\[V82SelectorRenderFrame\] "
    r"present=(?P<present>\d+) guest=(?P<guest>\d+) "
    r"generation=(?P<generation>\d+) stable=(?P<stable>\d+) "
    r"object=(?P<object>\S+) triangles=(?P<triangles>\d+) "
    r"reflection-opaque=(?P<opaque>\d+) "
    r"reflection-gloss=(?P<gloss>\d+) "
    r"area=(?P<area>[0-9.]+) "
    r"reflection-area=(?P<reflection_area>[0-9.]+) "
    r"max-area=(?P<max_area>[0-9.]+) "
    r"max-span=(?P<max_span_x>[0-9.]+),(?P<max_span_y>[0-9.]+) "
    r"materials=(?P<materials>.*)$",
    re.MULTILINE,
)
CREATED = re.compile(
    r"\[V82Vehicles\] created (?P<stable>guest\.v8\.[a-z0-9_]+) "
    r"native-selector object=(?P<object>\S+)"
)
RELEASED = re.compile(
    r"\[V82SelectorResources\] released=(?P<stable>\S+) "
    r"reason=(?P<reason>\S+) pc_allocations=(?P<allocations>\d+) "
    r"vram_reservations=(?P<vram>\d+) "
    r"selector_live_pointers=(?P<pointers>\d+)"
)
TEARDOWN = re.compile(
    r"\[V82Vehicles\] selector teardown retired "
    r"(?P<descriptors>\d+) host descriptors, released "
    r"(?P<released>\d+) independent native VRAM rectangles, observed "
    r"(?P<retail>\d+) retail-owned releases"
)
PACKET_ARENA = re.compile(
    r"\[PacketArenaRetire\] gameplay=(?P<gameplay>[01]) "
    r"range=(?P<start>\S+)\.\.(?P<end>\S+) .*"
    r"reflection-packets=(?P<before>\d+)->(?P<after>\d+)"
)
FATAL = re.compile(
    r"\[fatal\]|unhandled exception|unmapped call|out of vram|"
    r"outofmemoryexception|selector backing VRAM rectangle .*already released",
    re.IGNORECASE,
)


def parse_materials(value: str) -> dict[str, int]:
    result: dict[str, int] = {}
    if not value:
        return result
    for field in value.split(","):
        name, count = field.rsplit(":", 1)
        result[name] = int(count)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--minimum-generations",
        type=int,
        default=3,
        help="minimum preview constructions required for every revisited guest",
    )
    args = parser.parse_args()

    text = args.log.read_text(encoding="utf-8", errors="replace")
    frames: list[dict[str, object]] = []
    by_generation: dict[tuple[int, int], list[dict[str, object]]] = (
        defaultdict(list)
    )
    for match in FRAME.finditer(text):
        row: dict[str, object] = {
            "present": int(match["present"]),
            "guest": int(match["guest"]),
            "generation": int(match["generation"]),
            "stable_frame": int(match["stable"]),
            "object": match["object"],
            "triangles": int(match["triangles"]),
            "reflection_opaque": int(match["opaque"]),
            "reflection_gloss": int(match["gloss"]),
            "area": float(match["area"]),
            "reflection_area": float(match["reflection_area"]),
            "max_area": float(match["max_area"]),
            "max_span": [
                float(match["max_span_x"]),
                float(match["max_span_y"]),
            ],
            "materials": parse_materials(match["materials"]),
        }
        frames.append(row)
        by_generation[(row["guest"], row["generation"])].append(row)

    generations: list[dict[str, object]] = []
    guest_generations: dict[int, set[int]] = defaultdict(set)
    for (guest, generation), rows in sorted(by_generation.items()):
        guest_generations[guest].add(generation)
        material_names = sorted({
            name
            for row in rows
            for name in row["materials"]
        })
        generations.append({
            "guest": guest,
            "generation": generation,
            "object": sorted({row["object"] for row in rows}),
            "present_frames": len(rows),
            "stable_range": [
                min(row["stable_frame"] for row in rows),
                max(row["stable_frame"] for row in rows),
            ],
            "triangle_range": [
                min(row["triangles"] for row in rows),
                max(row["triangles"] for row in rows),
            ],
            "opaque_reflection_triangles": sum(
                row["reflection_opaque"] for row in rows
            ),
            "gloss_reflection_triangles": sum(
                row["reflection_gloss"] for row in rows
            ),
            "max_triangle_area": max(row["max_area"] for row in rows),
            "max_span": [
                max(row["max_span"][0] for row in rows),
                max(row["max_span"][1] for row in rows),
            ],
            "materials": material_names,
        })

    created = [match.groupdict() for match in CREATED.finditer(text)]
    released = [
        {
            **match.groupdict(),
            "pc_allocations": int(match["allocations"]),
            "vram_reservations": int(match["vram"]),
            "selector_live_pointers": int(match["pointers"]),
        }
        for match in RELEASED.finditer(text)
    ]
    teardowns = [
        {name: int(value) for name, value in match.groupdict().items()}
        for match in TEARDOWN.finditer(text)
    ]
    fatal_hits = [
        line for line in text.splitlines() if FATAL.search(line)
    ]
    revisited = {
        guest: sorted(values)
        for guest, values in sorted(guest_generations.items())
        if len(values) > 1
    }
    packet_arenas = [
        {
            "gameplay": bool(int(match["gameplay"])),
            "start": match["start"],
            "end": match["end"],
            "reflection_before": int(match["before"]),
            "reflection_after": int(match["after"]),
        }
        for match in PACKET_ARENA.finditer(text)
    ]
    generation_rows_by_guest: dict[int, list[dict[str, object]]] = (
        defaultdict(list)
    )
    for item in generations:
        generation_rows_by_guest[item["guest"]].append(item)
    role_signature_stable = all(
        len({
            (
                item["opaque_reflection_triangles"] > 0,
                item["gloss_reflection_triangles"] > 0,
            )
            for item in items
        }) == 1
        for items in generation_rows_by_guest.values()
    )
    material_signature_stable = all(
        len({tuple(item["materials"]) for item in items}) == 1
        for items in generation_rows_by_guest.values()
    )
    checks = {
        "render_frames_observed": bool(frames),
        "multiple_guests_revisited": len(revisited) >= 3,
        "minimum_generations_reached": bool(revisited) and all(
            len(values) >= args.minimum_generations
            for values in revisited.values()
        ),
        "opaque_reflection_role_observed": bool(generations) and all(
            item["opaque_reflection_triangles"] > 0
            for item in generations
        ),
        "semitransparent_gloss_role_observed": any(
            item["gloss_reflection_triangles"] > 0
            for item in generations
        ),
        "reflection_roles_stable_across_rebuilds":
            bool(generation_rows_by_guest) and role_signature_stable,
        "material_roles_stable_across_rebuilds":
            bool(generation_rows_by_guest) and material_signature_stable,
        "frontend_packet_arenas_retire_reflection_metadata": any(
            not item["gameplay"] and
            item["reflection_before"] > item["reflection_after"]
            for item in packet_arenas
        ),
        "selector_resources_retire_cleanly": bool(released) and all(
            item["vram_reservations"] == 0 and
            item["selector_live_pointers"] == 0
            for item in released
        ),
        "teardown_balances_every_finished_preview": (
            len(teardowns) >= max(0, len(created) - 1)
        ),
        "no_runtime_failure_markers": not fatal_hits,
        "deterministic_replay_completed": (
            "deterministic replay completed" in text
        ),
    }
    report = {
        "schema": 1,
        "passed": all(checks.values()),
        "checks": checks,
        "frame_count": len(frames),
        "created_preview_count": len(created),
        "released_preview_count": len(released),
        "teardown_count": len(teardowns),
        "packet_arena_retirement_count": len(packet_arenas),
        "revisited_guests": revisited,
        "generations": generations,
        "fatal_hits": fatal_hits,
        "release_tail": released[-8:],
        "teardown_tail": teardowns[-8:],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[SelectorRenderChurn] {'PASS' if report['passed'] else 'FAIL'} "
        f"frames={len(frames)} previews={len(created)} "
        f"revisited={','.join(map(str, revisited))}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"  report={args.output.resolve()}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
