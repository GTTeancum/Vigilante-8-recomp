#!/usr/bin/env python3
"""Single-process hidden/silent renderer gate for every playable V8:2 map."""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import os
import re
import subprocess
import time
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-map-fidelity-gate"

# A map may select its authored resources through the registry, but shipping
# rendering must never recognize a particular arena.  Keep this release-gate
# audit limited to the Enhanced renderer and its presentation compositor so
# content registries and test orchestration remain free to name assets.
RENDERER_AUDIT_PATHS = (
    REPO / "tools" / "recompone-reference" / "RecompOne.Runtime" /
    "Gpu" / "Enhanced",
    REPO / "tools" / "recompone-reference" / "RecompOne.Runtime" /
    "Host" / "Window" / "PresentationRenderer.cs",
)
MAP_SPECIFIC_RENDERER_TOKENS = (
    "route66", "olympic", "bayou", "launch", "steelmil", "nuclear",
    "oilfield", "harbor", "scrtbase", "sandfact", "airgrave",
    "wildwest", "hoovrdam", "vallyfrm", "casnocty", "canynlnd",
    "skiresrt", "dreamlnd", "dreamland", "florida",
)

MAPS = (
    ("LEVELS_ROUTE66", "levels/route66"),
    ("LEVELS_OLYMPIC", "levels/olympic"),
    ("LEVELS_BAYOU", "levels/bayou"),
    ("LEVELS_LAUNCH", "levels/launch"),
    ("LEVELS_STEELMIL", "levels/steelmil"),
    ("LEVELS_NUCLEAR", "levels/nuclear"),
    ("LEVELS_OILFIELD", "levels/oilfield"),
    ("LEVELS_HARBOR", "levels/harbor"),
    ("LEVELS_V8_SCRTBASE", "levels/v8/scrtbase"),
    ("LEVELS_V8_SANDFACT", "levels/v8/sandfact"),
    ("LEVELS_V8_OILFIELD", "levels/v8/oilfield"),
    ("LEVELS_V8_AIRGRAVE", "levels/v8/airgrave"),
    ("LEVELS_V8_WILDWEST", "levels/v8/wildwest"),
    ("LEVELS_V8_HOOVRDAM", "levels/v8/hoovrdam"),
    ("LEVELS_V8_VALLYFRM", "levels/v8/vallyfrm"),
    ("LEVELS_V8_CASNOCTY", "levels/v8/casnocty"),
    ("LEVELS_V8_CANYNLND", "levels/v8/canynlnd"),
    ("LEVELS_V8_SKIRESRT", "levels/v8/skiresrt"),
    (
        "LEVELS_N64_DREAMLND",
        "mods/v82_n64_super_dreamland/files/levels/n64/dreamlnd",
    ),
)

# Test-only controller route used after each gameplay-stage latch.  The route
# is deliberately independent of arena identity: test orchestration chooses a
# map, while the shipping renderer and game receive only ordinary controller
# input.  Sustained left/right sweeps expose horizon, close-scenery and terrain
# transitions; the short fire pulses exercise particles, translucency and
# depth composition without enabling the soak's separate steering automation.
ROUTE_PULSES = (
    ("departure", 30, 180, ("CROSS",)),
    ("left_close_sweep", 210, 180, ("CROSS", "LEFT")),
    ("mid_traversal", 390, 180, ("CROSS",)),
    ("right_close_sweep", 570, 180, ("CROSS", "RIGHT")),
    ("far_traversal", 750, 180, ("CROSS",)),
    ("brake_observation", 930, 60, ("SQUARE",)),
    ("counter_sweep", 990, 180, ("CROSS", "LEFT")),
    ("effect_r2_a", 180, 3, ("R2",)),
    ("effect_l2_a", 300, 3, ("L2",)),
    ("effect_r2_b", 480, 3, ("R2",)),
    ("effect_l2_b", 660, 3, ("L2",)),
    ("effect_r2_c", 840, 3, ("R2",)),
    ("effect_l2_c", 1080, 3, ("L2",)),
)

POSITION_RE = re.compile(
    r"\[V82Soak\] gameplay frame=(\d+).*?"
    r"pos=\((-?\d+),(-?\d+),(-?\d+)\)"
)
FOG_RE = re.compile(
    r"\[EnhancedFogFrame\].*?frame=(\d+) tick=(\d+) valid=(\d+) "
    r"reset-frame=(-?\d+) selected-frame=(-?\d+).*?span=([0-9.]+) "
    r"near-white=(\d+).*?rgb="
    r"([0-9.]+),([0-9.]+),([0-9.]+)"
)
TERRAIN_FRAME_RE = re.compile(
    r"\[TerrainFrame\].*?cells=(\d+) emitted=(\d+).*?"
    r"nearReject=(\d+).*?poolDropped=(\d+).*?flagErrors=(\d+)"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def renderer_content_branch_hits() -> list[dict[str, object]]:
    """Return literal arena identities found in shipping rendering sources."""
    source_files: list[Path] = []
    for path in RENDERER_AUDIT_PATHS:
        if path.is_dir():
            source_files.extend(sorted(path.rglob("*.cs")))
        elif path.is_file():
            source_files.append(path)
    hits: list[dict[str, object]] = []
    for path in source_files:
        for line_number, line in enumerate(
            path.read_text(encoding="utf-8", errors="replace").splitlines(),
            start=1,
        ):
            folded = line.casefold()
            tokens = sorted({
                token for token in MAP_SPECIFIC_RENDERER_TOKENS
                if token in folded
            })
            if tokens:
                hits.append({
                    "file": str(path.relative_to(REPO)),
                    "line": line_number,
                    "tokens": tokens,
                    "text": line.strip(),
                })
    return hits


def active_route_pulses(gameplay_frames: int) -> tuple[tuple, ...]:
    return tuple(
        (name, start, min(duration, gameplay_frames - start), buttons)
        for name, start, duration, buttons in ROUTE_PULSES
        if start < gameplay_frames
    )


def fixture_text(
    gameplay_frames: int,
    maps: tuple = MAPS,
    capture_gameplay: bool = False,
    capture_poll: int = 601,
) -> str:
    lines = [
        "# Initial boot/FMV/title/one-player Arcade path.",
        "540+3=START",
        "900+3=START",
        "1020+2=SELECT",
        "1080+3=CROSS",
        "1200+2=SELECT",
        "1260+3=CROSS",
        "1380+2=SELECT",
        "",
    ]
    # shell_transition visit 1 enters the first selector. Each completed match
    # returns on the next visit; replay the menu path for matches 2..19.
    for visit in range(2, len(maps) + 1):
        lines.extend([
            f"[after:shell_transition@{visit}]",
            "540+3=START",
            "660+2=SELECT",
            "720+3=CROSS",
            "",
        ])
    for generation in range(1, len(maps) + 1):
        lines.extend([
            f"[choose_location_{generation}]",
            "1250+3=CROSS",
            "",
        ])
    route = active_route_pulses(gameplay_frames)
    for generation in range(1, len(maps) + 1):
        lines.append(f"[after:gameplay@{generation}]")
        for name, start, duration, buttons in route:
            lines.append(
                f"{start}+{duration}={','.join(buttons)} # {name}"
            )
        lines.append("")
    if capture_gameplay:
        # The ordinary stage capture fires while some arenas are still running
        # their authored title camera. A unique direct-stage pulse requests one
        # composed proof later, after the intro has settled, without producing
        # a burst or changing shipping/runtime behavior.
        lines.extend([
            "[gameplay]",
            f"{capture_poll}+1=CROSS # representative_visual_capture",
            "",
        ])
    lines.extend([
        "[choose_player]",
        "650+3=CROSS",
        "900+3=CROSS",
        "1150+3=CROSS",
        "",
        "[choose_enemies]",
        "220+3=CROSS",
        "470+3=CROSS",
        "",
        "[defeated]",
    ])
    lines.extend([
        "700+2=SELECT",
        "1000+30=CROSS",
        "1400+12=UP",
        "1600+12=DOWN",
        "1800+30=CROSS",
    ])
    return "\n".join(lines) + "\n"


def game_process_ids() -> list[int]:
    """Return every active Vigilante82PC process without opening a window."""
    if os.name != "nt":
        return []
    completed = subprocess.run(
        [
            "tasklist", "/FI", "IMAGENAME eq Vigilante82PC.exe",
            "/FO", "CSV", "/NH",
        ],
        capture_output=True,
        text=True,
        check=False,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
    )
    if completed.returncode != 0:
        return []
    result: list[int] = []
    for row in csv.reader(completed.stdout.splitlines()):
        if len(row) < 2 or row[0].lower() != "vigilante82pc.exe":
            continue
        try:
            result.append(int(row[1]))
        except ValueError:
            pass
    return result


def route_summary(segment: str, expected_pulses: int) -> dict:
    points = [
        (int(frame), int(x), int(y), int(z))
        for frame, x, y, z in POSITION_RE.findall(segment)
    ]
    unique = len({(x, y, z) for _frame, x, y, z in points})
    path_distance = sum(
        math.hypot(b[1] - a[1], b[3] - a[3])
        for a, b in zip(points, points[1:])
    )
    if points:
        xs = [point[1] for point in points]
        ys = [point[2] for point in points]
        zs = [point[3] for point in points]
        horizontal_span = math.hypot(max(xs) - min(xs), max(zs) - min(zs))
        displacement = math.hypot(
            points[-1][1] - points[0][1],
            points[-1][3] - points[0][3],
        )
        frame_range = [points[0][0], points[-1][0]]
        axis_span = {
            "x": max(xs) - min(xs),
            "y": max(ys) - min(ys),
            "z": max(zs) - min(zs),
        }
    else:
        horizontal_span = displacement = 0.0
        frame_range = []
        axis_span = {"x": 0, "y": 0, "z": 0}
    pulse_count = len(re.findall(
        r"\[Input\] scripted pulse at after stage 'gameplay@\d+'",
        segment,
    ))
    fog_samples = [
        (
            int(frame), int(tick), int(valid), int(reset_frame),
            int(selected_frame), float(span), int(near_white),
            (float(red), float(green), float(blue)),
        )
        for (
            frame, tick, valid, reset_frame, selected_frame, span, near_white,
            red, green, blue,
        ) in FOG_RE.findall(segment)
    ]
    valid_fog = [sample for sample in fog_samples if sample[2] == 1]
    fog_spans = [sample[5] for sample in valid_fog]
    cross_map_stale_fog = [
        sample for sample in valid_fog
        if sample[4] < sample[3]
    ]
    fog_candidate_ages = [
        sample[0] - sample[4] for sample in valid_fog
    ]
    fog_rgb = sorted({
        tuple(round(channel, 6) for channel in sample[7])
        for sample in valid_fog
    })
    terrain_samples = [
        tuple(int(value) for value in values)
        for values in TERRAIN_FRAME_RE.findall(segment)
    ]
    pool_dropped_max = max(
        (sample[3] for sample in terrain_samples), default=0
    )

    def value_range(values: list[int] | list[float]) -> list[int] | list[float]:
        return [min(values), max(values)] if values else []

    return {
        "position_samples": len(points),
        "unique_positions": unique,
        "frame_range": frame_range,
        "axis_span": axis_span,
        "horizontal_span": round(horizontal_span, 3),
        "horizontal_path_distance": round(path_distance, 3),
        "start_to_end_displacement": round(displacement, 3),
        "scripted_route_pulses": pulse_count,
        "expected_route_pulses": expected_pulses,
        "renderer_windows": segment.count("[EnhancedRenderer] frames="),
        "terrain_frames": segment.count("[TerrainFrame]"),
        "fog_frames": segment.count("[EnhancedFogFrame]"),
        "framebuffer_changes": len(set(re.findall(
            r"\[GPU\] framebuffer .*?hash=(0x[0-9A-Fa-f]+)", segment
        ))),
        "weapon_armed": "[V82Coverage] weapon-armed" in segment,
        "weapon_fired": "[V82Coverage] weapon-fired" in segment,
        "fog_profile": {
            "samples": len(fog_samples),
            "valid_samples": len(valid_fog),
            "near_white_samples": sum(sample[6] for sample in valid_fog),
            "span_range": value_range(fog_spans),
            "reset_frame_range": value_range([
                sample[3] for sample in fog_samples
            ]),
            "candidate_age_range": value_range(fog_candidate_ages),
            "cross_map_stale_samples": len(cross_map_stale_fog),
            "unique_rgb": [list(rgb) for rgb in fog_rgb],
        },
        "terrain_profile": {
            "samples": len(terrain_samples),
            "cell_range": value_range([sample[0] for sample in terrain_samples]),
            "emitted_range": value_range([
                sample[1] for sample in terrain_samples
            ]),
            "near_reject_range": value_range([
                sample[2] for sample in terrain_samples
            ]),
            "pool_dropped_range": value_range([
                sample[3] for sample in terrain_samples
            ]),
            "pool_dropped_delta": (
                max(sample[3] for sample in terrain_samples) -
                min(sample[3] for sample in terrain_samples)
                if terrain_samples else 0
            ),
            "pool_dropped_max": pool_dropped_max,
            "flag_errors_range": value_range([
                sample[4] for sample in terrain_samples
            ]),
            "flag_errors_delta": (
                max(sample[4] for sample in terrain_samples) -
                min(sample[4] for sample in terrain_samples)
                if terrain_samples else 0
            ),
        },
        "passed": (
            len(points) >= 4 and unique >= 4 and
            horizontal_span >= 0x40000 and
            pulse_count >= expected_pulses and
            pool_dropped_max == 0 and
            not cross_map_stale_fog and
            "[V82Coverage] weapon-armed" in segment and
            "[V82Coverage] weapon-fired" in segment and
            "[EnhancedRenderer] frames=" in segment
        ),
    }


def location_token(slot: int) -> str:
    return "n64.super_dreamland_64" if slot == 18 else f"retail.{slot}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--gameplay-frames",
        type=int,
        default=360,
        help="representative gameplay cadence per map (default: 360)",
    )
    map_selection = parser.add_mutually_exclusive_group()
    map_selection.add_argument(
        "--map-count", type=int,
        help="prefix of the 19-map release roster to exercise (default: all)",
    )
    map_selection.add_argument(
        "--map-slots",
        help="comma-separated native arena slots to exercise in that order",
    )
    parser.add_argument("--timeout", type=float, default=7200.0)
    parser.add_argument("--unthrottled", action="store_true")
    parser.add_argument(
        "--capture-gameplay",
        action="store_true",
        help="retain exactly one composed gameplay frame for visual review",
    )
    parser.add_argument(
        "--capture-gameplay-poll",
        type=int,
        default=601,
        help=(
            "stage-relative input poll for the one visual proof "
            "(default: 601, after the arena title camera on the full "
            "presentation parity)"
        ),
    )
    parser.add_argument(
        "--verbose-render-log",
        action="store_true",
        help=(
            "record per-glyph provenance and per-frame/per-cell terrain "
            "selection, shading, texture-atlas and winding diagnostics"
        ),
    )
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    output = args.output.resolve()
    if not exe.is_file():
        parser.error(f"executable not found: {exe}")
    if not (loose / "SYSTEM.CNF").is_file():
        parser.error(f"loose root lacks SYSTEM.CNF: {loose}")
    if args.gameplay_frames <= 0:
        parser.error("gameplay frames must be positive")
    if args.capture_gameplay and not (
        0 < args.capture_gameplay_poll < args.gameplay_frames * 2
    ):
        parser.error(
            "capture gameplay poll must be between 1 and twice the "
            "gameplay-frame count"
        )
    if args.map_slots:
        try:
            selected_slots = [
                int(value.strip()) for value in args.map_slots.split(",")
                if value.strip()
            ]
        except ValueError:
            parser.error("map slots must be comma-separated integers")
        if not selected_slots:
            parser.error("map slots cannot be empty")
        if len(set(selected_slots)) != len(selected_slots):
            parser.error("map slots must be unique")
        invalid_slots = [
            slot for slot in selected_slots if slot < 0 or slot >= len(MAPS)
        ]
        if invalid_slots:
            parser.error(
                f"map slots must be between 0 and {len(MAPS) - 1}: " +
                ",".join(str(slot) for slot in invalid_slots)
            )
    else:
        map_count = len(MAPS) if args.map_count is None else args.map_count
        if map_count <= 0 or map_count > len(MAPS):
            parser.error(f"map count must be between 1 and {len(MAPS)}")
        selected_slots = list(range(map_count))
    selected_maps = tuple(MAPS[slot] for slot in selected_slots)
    output.mkdir(parents=True, exist_ok=True)
    fixture = output / "input.txt"
    runtime_log = output / "runtime.log"
    stdout_log = output / "stdout.log"
    stderr_log = output / "stderr.log"
    acceptance = output / "acceptance.json"
    for stale in (runtime_log, stdout_log, stderr_log, acceptance):
        stale.unlink(missing_ok=True)
    for stale_image in output.glob("*.ppm"):
        stale_image.unlink()
    fixture.write_text(
        fixture_text(
            args.gameplay_frames,
            selected_maps,
            capture_gameplay=args.capture_gameplay,
            capture_poll=args.capture_gameplay_poll,
        ),
        encoding="utf-8",
    )
    exe_hash_before = sha256(exe)

    preexisting_processes = game_process_ids()
    if preexisting_processes:
        parser.error(
            "refusing to start while Vigilante82PC is already running: " +
            ",".join(str(pid) for pid in preexisting_processes)
        )

    env = os.environ.copy()
    env.update({
        "RECOMPONE_INPUT_FILE": str(fixture),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_GPU_HLE": "1",
        "RECOMPONE_GRAPHICS_PRESET": "Enhanced",
        "RECOMPONE_MUTE": "1",
        "SDL_AUDIODRIVER": "dummy",
        "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_UNTHROTTLED": "1" if args.unthrottled else "0",
        "RECOMPONE_TRACE_INPUT": "1",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_SCRIPT_STAGE_CAPTURE_FILTER": "",
        "RECOMPONE_TRACE_LOADING_UI_TEXTURES": "1",
        "RECOMPONE_TRACE_FONT_TEXTURES": "1",
        "RECOMPONE_TRACE_MDEC": "1",
        "RECOMPONE_TRACE_V82_ARENA_SELECTOR": "1",
        "RECOMPONE_TRACE_ENHANCED_RENDERER": "1",
        "RECOMPONE_TRACE_FOG": "1",
        "RECOMPONE_TRACE_HUD": "1",
        "RECOMPONE_V82_TRACE_TERRAIN_FRAME": "1",
        "RECOMPONE_DISPLAY_PROBE_INTERVAL": "60",
        "RECOMPONE_DISPLAY_PROBE_IMAGES": "0",
        "RECOMPONE_LOG_PATH": str(runtime_log),
        "RECOMPONE_MOD_DIR": str(loose / "mods"),
        "RECOMPONE_V82_SOAK": "1",
        "RECOMPONE_V82_UNLOCK_ROSTER": "1",
        "RECOMPONE_V82_SOAK_POWERUPS": "0",
        "RECOMPONE_V82_SOAK_WEAPONS": "1",
        "RECOMPONE_V82_SOAK_NO_AUTOINPUT": "1",
        "RECOMPONE_V82_SOAK_WEAPON_AUTOINPUT": "1",
        "RECOMPONE_SOAK_HEARTBEAT_FRAMES": "30",
        "RECOMPONE_SOAK_TEARDOWN_FRAMES": "0",
        "RECOMPONE_V82_TEST_DEFEAT_FRAME": str(args.gameplay_frames),
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE": "shell_transition",
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE_VISITS": str(len(selected_maps) + 1),
        "RECOMPONE_V82_ARENA_SLOT_SEQUENCE": ",".join(
            str(slot) for slot in selected_slots
        ),
        "RECOMPONE_V82_PLAYER_TYPE_SEQUENCE": ",".join(
            "0" for _ in selected_maps
        ),
        "RECOMPONE_PRESENTATION_CAPTURE": (
            "1" if args.capture_gameplay else "0"
        ),
        "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
        "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "0",
        "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
        "RECOMPONE_CAPTURE_SELECTOR_GENERATIONS": "0",
        "RECOMPONE_CAPTURE_DIR": str(output),
    })
    if args.verbose_render_log:
        env.update({
            "RECOMPONE_V82_TRACE_TERRAIN_CELLS": "1",
            "RECOMPONE_TRACE_TERRAIN_DETAIL": "1",
            "RECOMPONE_TRACE_TERRAIN_SHADE": "1",
            "RECOMPONE_TRACE_TERRAIN_ATLAS_FRAGMENTS": "1",
            "RECOMPONE_TRACE_PACKET_NCLIP_CULL": "1",
            "RECOMPONE_TRACE_NEAR_CLIP": "1",
        })
    if args.capture_gameplay:
        env["RECOMPONE_CAPTURE_SCRIPTED_STAGE"] = "gameplay"
    else:
        env.pop("RECOMPONE_CAPTURE_SCRIPTED_STAGE", None)
    env.pop("RECOMPONE_ORIGINAL_RENDERER_ORACLE", None)
    for key in (
        "RECOMPONE_HEADLESS",
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS",
        "RECOMPONE_PRESENTATION_CAPTURE_FRAMES",
        "RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES",
        "RECOMPONE_V82_PLAYER_TYPE",
    ):
        env.pop(key, None)

    command = [str(exe), "--loose", str(loose)]
    started = time.monotonic()
    with stdout_log.open("wb") as stdout, stderr_log.open("wb") as stderr:
        process = subprocess.Popen(
            command,
            cwd=loose,
            env=env,
            stdout=stdout,
            stderr=stderr,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        observed_gameplay = 0
        maximum_game_processes = 0
        second_process_detected = False
        unexpected_gameplay = False
        scan_offset = 0
        scan_pending = ""
        next_process_scan = 0.0
        try:
            while process.poll() is None:
                now = time.monotonic()
                if now >= next_process_scan:
                    active_game_processes = game_process_ids()
                    maximum_game_processes = max(
                        maximum_game_processes, len(active_game_processes)
                    )
                    if any(pid != process.pid for pid in active_game_processes):
                        second_process_detected = True
                        process.terminate()
                        process.wait(timeout=10)
                        break
                    next_process_scan = now + 5.0
                if now - started > args.timeout:
                    process.kill()
                    process.wait(timeout=10)
                    break
                if runtime_log.is_file():
                    with runtime_log.open("rb") as stream:
                        stream.seek(scan_offset)
                        appended = stream.read()
                        scan_offset = stream.tell()
                    if appended:
                        scan_pending += appended.decode(
                            "utf-8", errors="replace"
                        )
                        line_end = max(
                            scan_pending.rfind("\n"),
                            scan_pending.rfind("\r"),
                        )
                        if line_end >= 0:
                            scan_text = scan_pending[:line_end + 1]
                            scan_pending = scan_pending[line_end + 1:]
                        else:
                            scan_text = ""
                        count = observed_gameplay + scan_text.count(
                            "[Input] stage 'gameplay'"
                        )
                        if count > len(selected_maps):
                            unexpected_gameplay = True
                            process.terminate()
                            process.wait(timeout=10)
                            break
                        while observed_gameplay < count:
                            observed_gameplay += 1
                            print(
                                f"[MapFidelity] gameplay "
                                f"{observed_gameplay}/{len(selected_maps)} "
                                f"{selected_maps[observed_gameplay - 1][0]}",
                                flush=True,
                            )
                time.sleep(0.5)
        except KeyboardInterrupt:
            process.kill()
            process.wait(timeout=10)
            raise
        except Exception:
            if process.poll() is None:
                process.kill()
                process.wait(timeout=10)
            raise
        exit_code = process.returncode

    runtime_text = runtime_log.read_text(
        encoding="utf-8", errors="replace"
    ) if runtime_log.is_file() else ""
    text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in (runtime_log, stdout_log, stderr_log)
        if path.is_file()
    )
    exe_hash_after = sha256(exe)
    detail_samples = [
        int(value)
        for value in re.findall(
            r"\[V82TerrainDetailRange\].*?samples=(\d+)", text
        )
    ]
    arena_usage = [
        (int(used), int(total), float(percent))
        for used, total, percent in re.findall(
            r"\[V82PacketArenaUsage\] high-water=(\d+)/(\d+) "
            r"percent=([0-9.]+)", text
        )
    ]
    maximum_usage = max((entry[2] for entry in arena_usage), default=0.0)
    loading_font_hits = len(re.findall(
        r"\[V82LoadingFontResolve\].*?\bhit=1\b", text
    ))
    loading_font_misses = len(re.findall(
        r"\[V82LoadingFontResolve\].*?\bhit=0\b.*?\bsize=[1-9]\d*x\d+",
        text,
    ))
    terrain_source_samples = [
        tuple(int(value) for value in sample)
        for sample in re.findall(
            r"\[EnhancedRenderer\].*?route-source-textured=(\d+).*?"
            r"route-source-coarse=(\d+).*?"
            r"route-source-transition=(\d+).*?"
            r"route-source-unresolved=(\d+)",
            text,
        )
    ]
    renderer_content_hits = renderer_content_branch_hits()
    captured_images = sorted(output.glob("*.ppm"))
    route = active_route_pulses(args.gameplay_frames)
    gameplay_segments = re.split(
        r"\[Input\] stage 'gameplay' at absolute poll \d+",
        runtime_text,
    )[1:]
    route_summaries = [
        route_summary(segment, len(route))
        for segment in gameplay_segments[:len(selected_maps)]
    ]
    expected_detail_calls = len(selected_maps) * args.gameplay_frames
    logged_detail_floor = max(1, expected_detail_calls // 600 * 600)
    checks: dict[str, bool] = {
        "clean_exit": exit_code == 0,
        "enhanced_renderer_only": (
            "[Host] graphics preset=Enhanced" in text and
            "renderer=PS1 software" not in text and
            "graphics preset=Original" not in text and
            "hle=False" not in text
        ),
        "single_game_process": (
            not second_process_detected and maximum_game_processes <= 1
        ),
        "no_unexpected_gameplay": not unexpected_gameplay,
        "exact_executable": exe_hash_before == exe_hash_after,
        "all_gameplay_visits":
            runtime_text.count("[Input] stage 'gameplay'") ==
            len(selected_maps),
        "all_native_defeats": text.count(
            "[V82DefeatRegression] native lethal damage completed"
        ) >= len(selected_maps),
        "all_shell_visits": text.count(
            "[Input] stage 'shell_transition'"
        ) >= len(selected_maps) + 1,
        "deterministic_completion": (
            "deterministic replay completed after stage "
            f"'shell_transition' visit {len(selected_maps) + 1}"
        ) in text,
        "all_hd_loading_cards_loaded":
            "[TexturePack] loaded 19 loading card overlays" in text,
        "all_hd_loading_cards_presented": text.count(
            "[TexturePack] selected loading card overlay arena="
        ) >= len(selected_maps),
        "loading_font_replacements_used": loading_font_hits > 0,
        "loading_font_replacements_complete": loading_font_misses == 0,
        "all_terrain_hits": text.count(
            "[TexturePack] terrain atlas hit key="
        ) >= len(selected_maps),
        "terrain_detail_extended": bool(detail_samples),
        "terrain_detail_range_extended":
            max(detail_samples, default=0) >= logged_detail_floor,
        "terrain_route_source_coverage": (
            bool(terrain_source_samples) and
            sum(sum(sample[:3]) for sample in terrain_source_samples) > 0 and
            sum(sample[3] for sample in terrain_source_samples) == 0
        ),
        "packet_arena_headroom": maximum_usage < 85.0,
        "all_representative_routes": (
            len(route_summaries) == len(selected_maps) and
            all(summary["passed"] for summary in route_summaries)
        ),
        "no_edge_pool_drops": (
            len(route_summaries) == len(selected_maps) and
            all(
                summary["terrain_profile"]["pool_dropped_max"] == 0
                for summary in route_summaries
            )
        ),
        "no_fatal": not re.search(
            r"(?i)\[fatal\]|unhandled exception|unmapped call|"
            r"out of vram|packet arena.*(?:overflow|exhaust)|pc heap exhausted",
            text,
        ),
        "no_map_specific_renderer_branches": not renderer_content_hits,
    }
    if args.capture_gameplay:
        checks["exactly_one_gameplay_image"] = (
            len(captured_images) == 1 and
            "gameplay" in captured_images[0].name.casefold()
        )
    else:
        checks["no_images"] = not captured_images
    map_checks = []
    for generation, (slot, (arena, terrain)) in enumerate(
        zip(selected_slots, selected_maps)
    ):
        location = location_token(slot)
        entry = {
            "slot": slot,
            "arena": arena,
            "terrain": terrain,
            "location_selected": f"[V82Arena] location={location}" in text,
            "slot_sequence": (
                f"arena-slot-sequence generation={generation + 1} "
                f"index={generation} "
                f"slot={slot}"
            ) in text,
            "hd_loading_card":
                f"selected loading card overlay arena={arena}:" in text,
            "hd_terrain": f"active terrain atlas={terrain} " in text,
            "route": (
                route_summaries[generation]
                if generation < len(route_summaries)
                else {"passed": False}
            ),
        }
        entry["passed"] = all(
            value for key, value in entry.items()
            if key not in {"slot", "arena", "terrain", "route"}
        )
        entry["passed"] = entry["passed"] and entry["route"]["passed"]
        map_checks.append(entry)
    checks["all_map_contracts"] = all(entry["passed"] for entry in map_checks)
    passed = all(checks.values())
    report = {
        "schema": 2,
        "passed": passed,
        "executable": str(exe),
        "executable_sha256": exe_hash_after,
        "executable_sha256_before": exe_hash_before,
        "executable_unchanged": exe_hash_before == exe_hash_after,
        "gameplay_frames_per_map": args.gameplay_frames,
        "maps": len(selected_maps),
        "map_slots": selected_slots,
        "expected_terrain_detail_calls": expected_detail_calls,
        "terrain_detail_logged_floor": logged_detail_floor,
        "detail_sample_high_water": max(detail_samples, default=0),
        "packet_arena_maximum_percent": maximum_usage,
        "maximum_game_processes": maximum_game_processes,
        "renderer_content_branch_hits": renderer_content_hits,
        "capture_gameplay": args.capture_gameplay,
        "capture_gameplay_poll": (
            args.capture_gameplay_poll if args.capture_gameplay else None
        ),
        "captured_images": [str(path) for path in captured_images],
        "route_profile": [
            {
                "name": name,
                "start_poll": start,
                "duration_polls": duration,
                "buttons": list(buttons),
            }
            for name, start, duration, buttons in route
        ],
        "checks": checks,
        "map_checks": map_checks,
        "logs": {
            "runtime": str(runtime_log),
            "stdout": str(stdout_log),
            "stderr": str(stderr_log),
        },
    }
    acceptance.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    state = "PASS" if passed else "FAIL"
    failed = ",".join(name for name, ok in checks.items() if not ok)
    print(
        f"[MapFidelity] {state} maps={len(selected_maps)} "
        f"packet_high_water={maximum_usage:.2f}%" +
        (f" failed={failed}" if failed else ""),
        flush=True,
    )
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
