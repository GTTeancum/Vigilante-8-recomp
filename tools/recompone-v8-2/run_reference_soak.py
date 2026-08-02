#!/usr/bin/env python3
"""Deterministic multi-arena crash hunt for Vigilante 8: 2nd Offense."""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import time
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = (
    REPO
    / "reference-v8-2"
    / "generated"
    / "recompiled"
    / "bin"
    / "Release"
    / "net10.0"
    / "win-x64"
    / "Vigilante82PC.exe"
)
DEFAULT_CUE = (
    REPO
    / "V8_2_BINCUE"
    / "Vigilante 8 - 2nd Offensive [U] [SLUS-00868].cue"
)
DEFAULT_OUTPUT = REPO / "artifacts" / "reference-v8-2-soak"

# The first eight are the sequel arenas. The remaining ten become visible
# through the retail original-disc flag enabled only while the soak is active.
EXPECTED_OVERLAYS = [
    "LEVELS_ROUTE66",
    "LEVELS_OLYMPIC",
    "LEVELS_BAYOU",
    "LEVELS_LAUNCH",
    "LEVELS_STEELMIL",
    "LEVELS_NUCLEAR",
    "LEVELS_OILFIELD",
    "LEVELS_HARBOR",
    "LEVELS_V8_SCRTBASE",
    "LEVELS_V8_SANDFACT",
    "LEVELS_V8_OILFIELD",
    "LEVELS_V8_AIRGRAVE",
    "LEVELS_V8_WILDWEST",
    "LEVELS_V8_HOOVRDAM",
    "LEVELS_V8_VALLYFRM",
    "LEVELS_V8_CASNOCTY",
    "LEVELS_V8_CANYNLND",
    "LEVELS_V8_SKIRESRT",
]

EXPECTED_POWERUPS = [
    "radar-jammer",
    "repair-wrench",
    "shield",
    "transform-1",
    "transform-2",
    "transform-3",
    "weapon-upgrade",
]

FATAL_MARKERS = {
    "unhandled exception": "unhandled exception",
    "fatal error": "fatal runtime error",
    "unmapped call:": "unmapped recompiled call",
    "accessviolationexception": "host access violation",
    "outofmemoryexception": "host out of memory",
    "pc heap exhausted": "PC heap exhaustion",
    "object scheduler exceeded": "runaway object scheduler",
    "entered drawsync wait without": "DrawSync callback failure",
    "did not complete after": "asynchronous wait failure",
}


@dataclass
class RunResult:
    cycle: int
    slot: int
    character_slot: int
    expected_overlay: str
    actual_overlay: str | None
    shared_overlays: list[str]
    passed: bool
    reason: str
    wall_seconds: float
    gameplay_seconds: float
    last_frame: int
    heartbeats: int
    framebuffer_changes: int
    callbacks: int
    collision_stream_rejections: int
    weapon_armed: list[int]
    weapon_fired: list[int]
    special_commands: list[str]
    powerups: list[str]
    power_state_changes: int
    stdout_log: str
    stderr_log: str
    gameplay_capture: str | None
    final_capture: str | None
    gameplay_presentation: str | None
    final_presentation: str | None
    presentation_frames: list[str]
    transformation_captures: list[str]
    hang_stack: str | None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--cue", type=Path, default=DEFAULT_CUE)
    parser.add_argument(
        "--loose-root",
        type=Path,
        help="use a standalone extracted sequel asset tree; no CUE/BIN is opened",
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--maps",
        default="all",
        help="comma-separated zero-based menu slots, or 'all'",
    )
    parser.add_argument(
        "--characters",
        default="0",
        help="comma-separated zero-based character slots, or 'all' for all 18",
    )
    parser.add_argument(
        "--rotate-characters",
        action="store_true",
        help=(
            "pair each selected map with a different character slot instead of "
            "cross-producting maps and --characters"
        ),
    )
    parser.add_argument(
        "--guest-vehicle",
        help="select an independent vehicle by stable ID through the engine roster API",
    )
    parser.add_argument("--cycles", type=int, default=1)
    parser.add_argument(
        "--campaign-hours",
        type=float,
        help="continue cycling all selected maps until this wall-time budget expires",
    )
    parser.add_argument(
        "--frames",
        type=int,
        default=2700,
        help="gameplay frames per run; 2700 covers all seven attachment kinds",
    )
    parser.add_argument("--entry-timeout", type=float, default=180.0)
    parser.add_argument("--heartbeat-timeout", type=float, default=90.0)
    parser.add_argument(
        "--weapon-start-kind",
        type=int,
        default=1,
        choices=range(1, 8),
        help="first attachment kind, useful for reproducing a late sweep failure",
    )
    parser.add_argument(
        "--coverage-profile",
        choices=("combined", "weapons", "powerups"),
        default="combined",
        help=(
            "exercise everything together, or isolate weapon and power-up/"
            "transformation coverage to avoid cross-feature stress effects"
        ),
    )
    parser.add_argument("--stop-on-failure", action="store_true")
    parser.add_argument(
        "--capture-presentation",
        action="store_true",
        help="retain full-resolution postprocessed gameplay and final captures",
    )
    parser.add_argument(
        "--presentation-resolution",
        default="1280x720",
        help="output resolution used with --capture-presentation",
    )
    parser.add_argument(
        "--presentation-frames",
        help=(
            "comma-separated absolute presentation frames to retain; "
            "also enables presentation capture"
        ),
    )
    parser.add_argument(
        "--presentation-burst-frames",
        type=int,
        default=0,
        help=(
            "retain this many consecutive frames when the gameplay capture "
            "stage is reached (maximum 600)"
        ),
    )
    return parser.parse_args()


def selected_slots(spec: str) -> list[int]:
    if spec.strip().lower() == "all":
        return list(range(len(EXPECTED_OVERLAYS)))
    slots = []
    for raw in spec.split(","):
        slot = int(raw.strip())
        if slot < 0 or slot >= len(EXPECTED_OVERLAYS):
            raise SystemExit(
                f"map slot {slot} is outside 0-{len(EXPECTED_OVERLAYS) - 1}"
            )
        slots.append(slot)
    return slots


def selected_characters(spec: str) -> list[int]:
    if spec.strip().lower() == "all":
        return list(range(18))
    slots = []
    for raw in spec.split(","):
        slot = int(raw.strip())
        if slot < 0 or slot >= 18:
            raise SystemExit(f"character slot {slot} is outside 0-17")
        slots.append(slot)
    return slots


def build_input_script(slot: int, character_slot: int = 0) -> str:
    lines = [
        "# Generated by tools/recompone-v8-2/run_reference_soak.py",
        "540+3=START",
        "900+3=START",
        "1020+2=SELECT",
        "1080+3=CROSS",
        "1200+2=SELECT",
        "1260+3=CROSS",
        "1380+2=SELECT",
    ]
    for index in range(slot):
        lines.append(f"{1450 + index * 30}+2=RIGHT")
    lines.append("2020+3=CROSS")
    # The harness installs the requested native vehicle type through
    # RECOMPONE_V82_PLAYER_TYPE (or the guest registry API) at the actual
    # player-object creation seam.  Do not try to reach that type by counting
    # selector RIGHT presses: the integrated V8 roster deliberately extends
    # the retail carousel, and decoding every intermediate imported preview
    # can exceed the gameplay-entry timeout without adding gameplay coverage.
    # Accept the current native selector entry, then let the engine seam apply
    # the exact requested type.
    character_select_frame = 2220
    lines.extend(
        [
            f"{character_select_frame}+2=SELECT",
            f"{character_select_frame + 60}+3=CROSS",
            f"{character_select_frame + 180}+2=SELECT",
            f"{character_select_frame + 240}+3=CROSS",
            f"{character_select_frame + 360}+2=SELECT",
            f"{character_select_frame + 420}+3=CROSS",
            "[gameplay]",
            "",
        ]
    )
    return "\n".join(lines)


def combined_text(stdout_path: Path, stderr_path: Path) -> str:
    parts = []
    for path in (stdout_path, stderr_path):
        try:
            parts.append(path.read_text(encoding="utf-8", errors="replace"))
        except FileNotFoundError:
            pass
    return "\n".join(parts)


def stop_process(process: subprocess.Popen[bytes]) -> None:
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def capture_managed_stack(
    process: subprocess.Popen[bytes], output: Path, stem: str
) -> Path | None:
    tool = shutil.which("dotnet-stack")
    if tool is None:
        fallback = Path.home() / ".dotnet" / "tools" / "dotnet-stack.exe"
        if fallback.is_file():
            tool = str(fallback)
    if tool is None or process.poll() is not None:
        return None
    path = output / f"{stem}.hang-stack.txt"
    try:
        with path.open("wb") as stream:
            subprocess.run(
                [tool, "report", "--process-id", str(process.pid)],
                stdout=stream,
                stderr=subprocess.STDOUT,
                timeout=25,
                check=False,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
            )
        if path.stat().st_size:
            return path
    except (OSError, subprocess.TimeoutExpired):
        pass
    path.unlink(missing_ok=True)
    return None


def preserve_capture(
    directory: Path, output: Path, source_name: str, target_name: str, started: float
) -> Path | None:
    source = directory / source_name
    try:
        if source.stat().st_mtime < started - 1:
            return None
    except FileNotFoundError:
        return None
    target = output / target_name
    shutil.copy2(source, target)
    return target


def preserve_capture_glob(
    directory: Path, output: Path, pattern: str, target_name: str, started: float
) -> Path | None:
    sources = []
    for source in directory.glob(pattern):
        try:
            if source.stat().st_mtime >= started - 1:
                sources.append(source)
        except FileNotFoundError:
            pass
    if not sources:
        return None
    source = max(sources, key=lambda path: path.stat().st_mtime)
    target = output / target_name
    shutil.copy2(source, target)
    return target


def clean_capture_sources(directory: Path, started: float) -> None:
    for pattern in (
        "recompone_capture_*.ppm",
        "recompone_present_*.ppm",
        "recompone_vram_latest.ppm",
    ):
        for path in directory.glob(pattern):
            try:
                if path.stat().st_mtime >= started - 1:
                    path.unlink()
            except FileNotFoundError:
                pass


def run_one(
    exe: Path,
    source_args: list[str],
    output: Path,
    cycle: int,
    slot: int,
    frames: int,
    entry_timeout: float,
    heartbeat_timeout: float,
    weapon_start_kind: int,
    character_slot: int,
    guest_vehicle: str | None,
    coverage_profile: str,
    capture_presentation: bool,
    presentation_resolution: str,
    presentation_frames_spec: str | None,
    presentation_burst_frames: int,
) -> RunResult:
    expected = EXPECTED_OVERLAYS[slot]
    stem = (
        f"{cycle:02d}_c{character_slot:02d}_{slot:02d}_"
        f"{expected.lower()}"
    )
    fixture_path = output / f"{stem}.input.txt"
    stdout_path = output / f"{stem}.stdout.log"
    stderr_path = output / f"{stem}.stderr.log"
    fixture_path.write_text(
        build_input_script(slot, character_slot), encoding="utf-8"
    )

    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_INPUT_FILE": str(fixture_path.resolve()),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_GPU_HLE": os.environ.get(
                "RECOMPONE_GPU_HLE", "1"),
            "RECOMPONE_V82_SOAK": "1",
            "RECOMPONE_V82_UNLOCK_ROSTER": "1",
            "RECOMPONE_SOAK_HEARTBEAT_FRAMES": "180",
            "RECOMPONE_SOAK_TEARDOWN_FRAMES": str(frames),
            "RECOMPONE_V82_SOAK_START_KIND": str(weapon_start_kind),
            "RECOMPONE_V82_SOAK_POWERUPS": (
                "0" if coverage_profile == "weapons" else "1"
            ),
            "RECOMPONE_V82_SOAK_WEAPONS": (
                "0" if coverage_profile == "powerups" else "1"
            ),
            "RECOMPONE_V82_SOAK_CAPTURE_TRANSFORMS": (
                "1" if coverage_profile == "powerups" else "0"
            ),
            "RECOMPONE_DISPLAY_PROBE_INTERVAL": "600",
            "RECOMPONE_CAPTURE_SCRIPTED_STAGE": "gameplay",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_UNTHROTTLED": "1",
            "RECOMPONE_LOOSE_DIR": "0",
            # Parallel matrix workers share the staged loose-files directory.
            # Keep every worker's framebuffer and presentation captures in its
            # own result directory so "latest" probes cannot collide.
            "RECOMPONE_CAPTURE_DIR": str(output.resolve()),
        }
    )
    if guest_vehicle is None:
        env["RECOMPONE_V82_PLAYER_TYPE"] = str(character_slot)
    else:
        env.pop("RECOMPONE_V82_PLAYER_TYPE", None)
    if capture_presentation or presentation_frames_spec:
        env["RECOMPONE_PRESENTATION_CAPTURE"] = "1"
        env["RECOMPONE_PRESENTATION_RESOLUTION"] = presentation_resolution
    if presentation_burst_frames > 0:
        env["RECOMPONE_PRESENTATION_CAPTURE"] = "1"
        env["RECOMPONE_PRESENTATION_RESOLUTION"] = presentation_resolution
        env["RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES"] = str(
            min(presentation_burst_frames, 600)
        )
        env["RECOMPONE_PRESENTATION_CAPTURE_BURST_LABEL"] = "gameplay"
    if presentation_frames_spec:
        env["RECOMPONE_PRESENTATION_CAPTURE_FRAMES"] = (
            presentation_frames_spec
        )

    started = time.time()
    gameplay_started: float | None = None
    last_progress_at = started
    last_frame = 0
    reason = ""
    passed = False
    hang_stack: Path | None = None
    creationflags = getattr(subprocess, "CREATE_NO_WINDOW", 0)

    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        process = subprocess.Popen(
            [
                str(exe),
                *source_args,
                *(
                    ["--guest-vehicle", guest_vehicle]
                    if guest_vehicle is not None
                    else []
                ),
            ],
            cwd=str(exe.parent),
            env=env,
            stdout=stdout,
            stderr=stderr,
            creationflags=creationflags,
        )
        try:
            while True:
                now = time.time()
                text = combined_text(stdout_path, stderr_path)
                lower = text.lower()
                for marker, description in FATAL_MARKERS.items():
                    if marker in lower:
                        reason = description
                        break
                if reason:
                    break

                if gameplay_started is None and "[Input] stage 'gameplay'" in text:
                    gameplay_started = now
                    last_progress_at = now
                    print(f"[{stem}] gameplay reached", flush=True)

                frame_matches = re.findall(
                    r"\[V82Soak\] gameplay frame=(\d+)", text
                )
                observed = int(frame_matches[-1]) if frame_matches else 0
                if observed > last_frame:
                    last_frame = observed
                    last_progress_at = now
                if last_frame >= frames:
                    passed = True
                    reason = f"completed {last_frame} gameplay frames"
                    break

                return_code = process.poll()
                if return_code is not None:
                    reason = f"process exited early with code {return_code}"
                    break
                if gameplay_started is None and now - started > entry_timeout:
                    reason = "gameplay entry timeout"
                    hang_stack = capture_managed_stack(process, output, stem)
                    break
                if gameplay_started is not None and now - last_progress_at > heartbeat_timeout:
                    reason = "gameplay heartbeat timeout"
                    hang_stack = capture_managed_stack(process, output, stem)
                    break
                time.sleep(0.5)
        finally:
            stop_process(process)

    capture_source = output
    gameplay_capture = preserve_capture(
        capture_source,
        output,
        "recompone_capture_gameplay.ppm",
        f"{stem}.gameplay.ppm",
        started,
    )
    final_capture = preserve_capture(
        capture_source,
        output,
        "recompone_vram_latest.ppm",
        f"{stem}.final.ppm",
        started,
    )
    gameplay_presentation = preserve_capture_glob(
        capture_source,
        output,
        "recompone_present_gameplay_*.ppm",
        f"{stem}.gameplay-presentation.ppm",
        started,
    )
    final_presentation = preserve_capture_glob(
        capture_source,
        output,
        "recompone_present_soak_teardown_*.ppm",
        f"{stem}.final-presentation.ppm",
        started,
    )
    presentation_frames = []
    frame_sources = [
        *capture_source.glob("recompone_present_frame_*.ppm"),
        *capture_source.glob("recompone_present_gameplay_???_*.ppm"),
    ]
    for source in sorted(frame_sources):
        try:
            if source.stat().st_mtime < started - 1:
                continue
        except FileNotFoundError:
            continue
        target = output / f"{stem}.{source.name}"
        shutil.copy2(source, target)
        presentation_frames.append(str(target))
    transformation_captures = [
        str(path)
        for mode in range(1, 4)
        if (
            path := preserve_capture(
                capture_source,
                output,
                f"recompone_capture_transform_{mode}.ppm",
                f"{stem}.transform-{mode}.ppm",
                started,
            )
        )
    ]
    clean_capture_sources(capture_source, started)

    text = combined_text(stdout_path, stderr_path)
    overlays = re.findall(
        r"loaded (?:relocated )?overlay: (LEVELS_[A-Z0-9_]+)", text
    )
    actual = overlays[-1] if overlays else None
    shared_overlays = sorted(
        set(
            re.findall(
                r"loaded (?:relocated )?overlay: (SHARED_[A-Z0-9_]+)",
                text,
            )
        )
    )
    heartbeats = re.findall(r"\[V82Soak\] gameplay frame=(\d+)", text)
    framebuffer_hashes = set(
        re.findall(r"\[GPU\] framebuffer .*?hash=(0x[0-9A-Fa-f]+)", text)
    )
    callbacks = {
        value.upper()
        for value in re.findall(r"\[V82Coverage\] callback=0x([0-9A-Fa-f]+)", text)
    }
    collision_stream_rejections = len(
        re.findall(r"\[V82Collision\] rejected malformed shape stream", text)
    )
    weapon_armed = sorted(
        {
            int(value)
            for value in re.findall(
                r"\[V82Coverage\] weapon-armed kind=(\d+)", text
            )
        }
    )
    weapon_fired = sorted(
        {
            int(value)
            for value in re.findall(
                r"\[V82Coverage\] weapon-fired kind=(\d+)", text
            )
        }
    )
    special_commands = sorted(
        {
            f"{weapon}:{variant}"
            for weapon, variant in re.findall(
                r"\[V82Coverage\] special-command weapon=(\d+) variant=(\d+)",
                text,
            )
        }
    )
    powerups = sorted(
        set(re.findall(r"\[V82Coverage\] powerup=([a-z0-9-]+)", text))
    )
    power_states = len(re.findall(r"\[V82Coverage\] power-state ", text))

    if passed and actual != expected:
        passed = False
        reason = f"menu slot loaded {actual or 'no arena'}, expected {expected}"
    if passed and len(heartbeats) < 2:
        passed = False
        reason = "insufficient gameplay heartbeats"
    if passed and len(framebuffer_hashes) < 2:
        passed = False
        reason = "rendered framebuffer did not change"
    if passed and len(callbacks) < 8:
        passed = False
        reason = "insufficient gameplay callback coverage"
    if (
        passed
        and frames >= 2700
        and coverage_profile != "powerups"
        and weapon_armed != list(range(1, 8))
    ):
        passed = False
        reason = f"incomplete weapon attachment coverage: {weapon_armed}"
    if (
        passed
        and frames >= 2700
        and coverage_profile != "weapons"
        and powerups != EXPECTED_POWERUPS
    ):
        passed = False
        reason = f"incomplete power-up coverage: {powerups}"
    if (
        passed
        and frames >= 2700
        and coverage_profile == "powerups"
        and len(transformation_captures) != 3
    ):
        passed = False
        reason = (
            "incomplete transformation capture coverage: "
            f"{len(transformation_captures)}/3"
        )

    ended = time.time()
    return RunResult(
        cycle=cycle,
        slot=slot,
        character_slot=character_slot,
        expected_overlay=expected,
        actual_overlay=actual,
        shared_overlays=shared_overlays,
        passed=passed,
        reason=reason,
        wall_seconds=round(ended - started, 3),
        gameplay_seconds=round(
            0 if gameplay_started is None else ended - gameplay_started, 3
        ),
        last_frame=last_frame,
        heartbeats=len(heartbeats),
        framebuffer_changes=len(framebuffer_hashes),
        callbacks=len(callbacks),
        collision_stream_rejections=collision_stream_rejections,
        weapon_armed=weapon_armed,
        weapon_fired=weapon_fired,
        special_commands=special_commands,
        powerups=powerups,
        power_state_changes=power_states,
        stdout_log=str(stdout_path),
        stderr_log=str(stderr_path),
        gameplay_capture=str(gameplay_capture) if gameplay_capture else None,
        final_capture=str(final_capture) if final_capture else None,
        gameplay_presentation=(
            str(gameplay_presentation) if gameplay_presentation else None
        ),
        final_presentation=(
            str(final_presentation) if final_presentation else None
        ),
        presentation_frames=presentation_frames,
        transformation_captures=transformation_captures,
        hang_stack=str(hang_stack) if hang_stack else None,
    )


def write_summary(
    output: Path, args: argparse.Namespace, results: list[RunResult], started: float
) -> None:
    payload = {
        "schema": "v82-reference-soak-v1",
        "startedUtc": datetime.fromtimestamp(started, timezone.utc).isoformat(),
        "updatedUtc": datetime.now(timezone.utc).isoformat(),
        "requestedFrames": args.frames,
        "campaignHours": args.campaign_hours,
        "coverageProfile": args.coverage_profile,
        "runs": [asdict(result) for result in results],
        "totals": {
            "runs": len(results),
            "passed": sum(result.passed for result in results),
            "failed": sum(not result.passed for result in results),
            "distinctOverlays": sorted(
                {result.actual_overlay for result in results if result.actual_overlay}
            ),
            "distinctSharedOverlays": sorted(
                {
                    overlay
                    for result in results
                    for overlay in result.shared_overlays
                }
            ),
            "weaponKindsArmed": sorted(
                {kind for result in results for kind in result.weapon_armed}
            ),
            "weaponKindsFired": sorted(
                {kind for result in results for kind in result.weapon_fired}
            ),
            "specialCommands": sorted(
                {command for result in results for command in result.special_commands}
            ),
            "collisionStreamRejections": sum(
                result.collision_stream_rejections for result in results
            ),
            "powerups": sorted(
                {powerup for result in results for powerup in result.powerups}
            ),
        },
    }
    (output / "summary.json").write_text(
        json.dumps(payload, indent=2) + "\n", encoding="utf-8"
    )


def main() -> int:
    args = parse_args()
    exe = args.exe.resolve()
    if not exe.is_file():
        raise SystemExit(f"Vigilante82PC executable not found: {exe}")
    if args.loose_root:
        loose_root = args.loose_root.resolve()
        if not (loose_root / "SYSTEM.CNF").is_file():
            raise SystemExit(
                f"Standalone sequel root lacks SYSTEM.CNF: {loose_root}")
        source_args = ["--loose", str(loose_root)]
    else:
        cue = args.cue.resolve()
        if not cue.is_file():
            raise SystemExit(
                f"Vigilante 8: 2nd Offense CUE not found: {cue}")
        source_args = [str(cue)]
    if args.frames < 2700:
        print(
            "[soak] warning: fewer than 2700 frames cannot cover all seven "
            "attachment windows",
            flush=True,
        )
    if args.cycles <= 0 or args.frames <= 0:
        raise SystemExit("cycles and frames must be positive")
    if args.campaign_hours is not None and args.campaign_hours <= 0:
        raise SystemExit("campaign hours must be positive")
    if args.presentation_burst_frames < 0:
        raise SystemExit("presentation burst frames must be non-negative")

    slots = selected_slots(args.maps)
    characters = selected_characters(args.characters)
    if args.rotate_characters:
        run_pairs = [
            (slot, characters[index % len(characters)])
            for index, slot in enumerate(slots)
        ]
    else:
        run_pairs = [
            (slot, character_slot)
            for character_slot in characters
            for slot in slots
        ]
    output = (
        args.output.resolve()
        if args.output
        else DEFAULT_OUTPUT / datetime.now().strftime("%Y%m%d-%H%M%S")
    )
    output.mkdir(parents=True, exist_ok=True)
    started = time.time()
    deadline = (
        started + args.campaign_hours * 3600
        if args.campaign_hours is not None
        else None
    )
    results: list[RunResult] = []

    cycle = 1
    while cycle <= args.cycles or (
        deadline is not None and (cycle == 1 or time.time() < deadline)
    ):
        for slot, character_slot in run_pairs:
            if deadline is not None and cycle > 1 and time.time() >= deadline:
                break
            print(
                f"[soak] cycle={cycle} character={character_slot} slot={slot} "
                f"expected={EXPECTED_OVERLAYS[slot]} frames={args.frames}",
                flush=True,
            )
            result = run_one(
                exe,
                source_args,
                output,
                cycle,
                slot,
                args.frames,
                args.entry_timeout,
                args.heartbeat_timeout,
                args.weapon_start_kind,
                character_slot,
                args.guest_vehicle,
                args.coverage_profile,
                args.capture_presentation,
                args.presentation_resolution,
                args.presentation_frames,
                args.presentation_burst_frames,
            )
            results.append(result)
            write_summary(output, args, results, started)
            status = "PASS" if result.passed else "FAIL"
            print(
                f"[soak] {status} character={character_slot} slot={slot} "
                f"actual={result.actual_overlay} frame={result.last_frame} "
                f"weapons={result.weapon_armed} reason={result.reason}",
                flush=True,
            )
            if not result.passed and args.stop_on_failure:
                return 1
        cycle += 1
        if deadline is None and cycle > args.cycles:
            break

    return 0 if results and all(result.passed for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
