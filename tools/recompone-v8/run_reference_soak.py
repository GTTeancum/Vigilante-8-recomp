#!/usr/bin/env python3
"""Deterministic multi-map and split-screen soak runner for Vigilante8PC."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from datetime import datetime
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "BINCUE" / "Vigilante8PC.exe"
DEFAULT_CUE = REPO / "BINCUE" / "Vigilante 8 (USA).cue"
DEFAULT_OUTPUT = REPO / "reference" / "generated" / "soak"


@dataclass(frozen=True)
class Arena:
    key: str
    display: str
    stage: str
    overlay: str


ARENAS = [
    Arena("ski_resort", "Ski Resort", "location_ski_resort", "SKIRESRT"),
    Arena("canyonlands", "Canyonlands", "location_canyonlands", "CANYNLND"),
    Arena("casino_city", "Casino City", "location_casino_city", "CASNOCTY"),
    Arena("valley_farms", "Valley Farms", "location_valley_farms", "VALLYFRM"),
    Arena("hoover_dam", "Hoover Dam", "location_hoover_dam", "HOOVRDAM"),
    Arena("ghost_town", "Ghost Town", "location_ghost_town", "WILDWEST"),
    Arena("aircraft_graveyard", "Aircraft Graveyard", "location_aircraft_graveyard", "AIRGRAVE"),
    Arena("oil_fields", "Oil Fields", "location_oilfield", "OILFIELD"),
    Arena("sand_factory", "Sand Factory", "location_sand_factory", "SANDFACT"),
    Arena("secret_base", "Secret Base", "location_secret_base", "SCRTBASE"),
]
ARENA_BY_KEY = {arena.key: arena for arena in ARENAS}
BONUS_ARENAS = {"sand_factory", "secret_base"}

FATAL_MARKERS = {
    "unmapped call": "unmapped call",
    "unhandled exception": "unhandled exception",
    "vigilante 8 fatal error": "original fatal error",
    "audio init failed": "audio initialization failure",
    "window unavailable": "window initialization failure",
    "heap exhausted": "heap exhaustion",
    "terrain tile pointer is invalid": "terrain pointer failure",
    "invalid animation": "invalid animation pointer",
    "stack overflow": "stack overflow",
    "outofmemoryexception": "out of memory",
}


@dataclass
class RunResult:
    arena: str
    mode: str
    cycle: int
    passed: bool
    reason: str
    wall_seconds: float
    gameplay_seconds: float
    heartbeats: int
    last_gameplay_tick: int
    overlay_seen: bool
    audio_ready: bool
    framebuffer_changes: int
    source_overrides: int | None
    stdout_log: str
    stderr_log: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--cue", type=Path, default=DEFAULT_CUE)
    parser.add_argument(
        "--maps",
        default="all",
        help=(
            "Comma-separated arena keys, 'all' for the eight arenas selectable "
            "on the current save (default), or 'all-including-locked'."
        ),
    )
    parser.add_argument("--mode", choices=("arcade", "coop", "versus"), default="arcade")
    parser.add_argument("--seconds", type=float, default=120.0, help="Gameplay soak time per arena.")
    parser.add_argument("--entry-timeout", type=float, default=45.0)
    parser.add_argument("--heartbeat-timeout", type=float, default=15.0)
    parser.add_argument("--cycles", type=int, default=1)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--continue-on-failure", action="store_true")
    return parser.parse_args()


def selected_arenas(spec: str) -> list[Arena]:
    normalized = spec.strip().lower()
    if normalized == "all":
        return [arena for arena in ARENAS if arena.key not in BONUS_ARENAS]
    if normalized in {"all-including-locked", "all_including_locked"}:
        return list(ARENAS)
    result: list[Arena] = []
    for raw in spec.split(","):
        key = raw.strip().lower().replace("-", "_").replace(" ", "_")
        if key not in ARENA_BY_KEY:
            choices = ", ".join(arena.key for arena in ARENAS)
            raise SystemExit(f"unknown arena '{raw}'; expected one of: {choices}")
        result.append(ARENA_BY_KEY[key])
    return result


def location_script(target: Arena) -> list[str]:
    lines = ["[select_location]"]
    for poll in range(5, 105, 10):
        lines.append(f"{poll}+2=DOWN")
    lines.extend(["", f"[{target.stage}]", "1+2=CROSS", ""])
    return lines


def gameplay_script(mode: str, seconds: float) -> list[str]:
    polls = max(1200, int(seconds * 90) + 1200)
    lines = ["[gameplay]"]
    if mode == "arcade":
        lines.extend(
            [
                f"120+{polls}=P1:CROSS",
                f"180+{polls}=P1:R2",
                f"240+{polls}=P1:L2",
            ]
        )
        for start in range(300, polls, 480):
            direction = "RIGHT" if (start // 480) % 2 == 0 else "LEFT"
            lines.append(f"{start}+180=P1:{direction}")
    else:
        lines.extend(
            [
                f"120+{polls}=P1:CROSS,P2:CROSS",
                f"180+{polls}=P1:R2,P2:R2",
                f"240+{polls}=P1:L2,P2:L2",
            ]
        )
        for start in range(300, polls, 480):
            p1 = "RIGHT" if (start // 480) % 2 == 0 else "LEFT"
            p2 = "LEFT" if p1 == "RIGHT" else "RIGHT"
            lines.append(f"{start}+180=P1:{p1},P2:{p2}")
    lines.append("")
    return lines


def build_fixture(arena: Arena, mode: str, seconds: float) -> str:
    lines = [
        "# Generated by run_reference_soak.py",
        "10+2=START",
        "",
        "[press_start]",
        "1+2=START",
        "",
    ]
    if mode == "arcade":
        lines.extend(
            [
                "[main_menu]",
                "1+2=DOWN",
                "10+2=CROSS",
                "",
                "[player_count]",
                "5+2=CROSS",
                "",
                "[choose_player]",
                "5+2=CROSS",
                "",
            ]
        )
    else:
        direction = "DOWN" if mode == "coop" else "UP"
        lines.extend(
            [
                "[player_count]",
                "5+2=DOWN",
                "15+2=CROSS",
                "",
                "[two_player_mode]",
                f"10+2={direction}",
                "20+2=CROSS",
                "",
            ]
        )

    lines.extend(location_script(arena))
    if mode == "arcade":
        lines.extend(["[choose_enemies]", "5+2=CROSS", ""])
    else:
        lines.extend(
            [
                "[choose_players]",
                "5+2=P1:CROSS",
                "15+2=P2:RIGHT",
                "25+2=P2:CROSS",
                "45+2=P1:CROSS,P2:CROSS",
                "",
                "[choose_enemies]",
                "5+2=P1:CROSS",
                "",
            ]
        )
    lines.extend(gameplay_script(mode, seconds))
    return "\n".join(lines)


def combined_text(stdout_path: Path, stderr_path: Path) -> str:
    parts = []
    for path in (stdout_path, stderr_path):
        try:
            parts.append(path.read_text(encoding="utf-8", errors="replace"))
        except FileNotFoundError:
            pass
    return "\n".join(parts)


def cleanup_runtime_artifacts(directory: Path, started_at: float) -> None:
    patterns = ("recompone_capture_*.ppm", "recompone_vram_latest.ppm")
    for pattern in patterns:
        for path in directory.glob(pattern):
            try:
                if path.stat().st_mtime >= started_at - 1.0:
                    path.unlink()
            except FileNotFoundError:
                pass


def stop_process(process: subprocess.Popen[bytes]) -> None:
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def run_one(
    exe: Path,
    cue: Path,
    output: Path,
    arena: Arena,
    mode: str,
    cycle: int,
    seconds: float,
    entry_timeout: float,
    heartbeat_timeout: float,
) -> RunResult:
    stem = f"{cycle:02d}_{mode}_{arena.key}"
    fixture_path = output / f"{stem}.input.txt"
    stdout_path = output / f"{stem}.stdout.log"
    stderr_path = output / f"{stem}.stderr.log"
    fixture_path.write_text(build_fixture(arena, mode, seconds), encoding="utf-8")

    env = os.environ.copy()
    env["RECOMPONE_INPUT_FILE"] = str(fixture_path.resolve())
    env["RECOMPONE_DISABLE_LIVE_INPUT"] = "1"
    env["RECOMPONE_FORCE_PAD2_CONNECTED"] = "1" if mode != "arcade" else "0"
    env["RECOMPONE_WINDOW_VISIBLE"] = "0"
    env["RECOMPONE_V8_GAME_VOLUME"] = "0"
    env["RECOMPONE_SOAK_HEARTBEAT_TICKS"] = "180"
    env["RECOMPONE_DISPLAY_PROBE_INTERVAL"] = "60"
    env["RECOMPONE_TARGET_LOCATION"] = arena.display
    env["RECOMPONE_TRACE_LEVEL_LOAD"] = "1"

    creationflags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    started = time.time()
    gameplay_started: float | None = None
    last_heartbeat_at: float | None = None
    heartbeat_count = 0
    reason = ""
    passed = False

    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        process = subprocess.Popen(
            [str(exe), str(cue)],
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
                    last_heartbeat_at = now
                    print(f"[{stem}] gameplay reached", flush=True)

                heartbeat_matches = list(
                    re.finditer(r"\[Soak\] gameplay tick=(\d+)", text)
                )
                if len(heartbeat_matches) > heartbeat_count:
                    heartbeat_count = len(heartbeat_matches)
                    last_heartbeat_at = now

                return_code = process.poll()
                if return_code is not None:
                    reason = f"process exited early with code {return_code}"
                    break

                if gameplay_started is None:
                    if now - started > entry_timeout:
                        reason = "gameplay entry timeout"
                        break
                else:
                    gameplay_elapsed = now - gameplay_started
                    if gameplay_elapsed >= seconds:
                        passed = True
                        reason = "completed requested gameplay duration"
                        break
                    if (
                        last_heartbeat_at is not None
                        and now - last_heartbeat_at > heartbeat_timeout
                    ):
                        reason = "gameplay heartbeat timeout"
                        break

                time.sleep(0.5)
        finally:
            stop_process(process)
            cleanup_runtime_artifacts(exe.parent, started)

    text = combined_text(stdout_path, stderr_path)
    heartbeats = [int(value) for value in re.findall(r"\[Soak\] gameplay tick=(\d+)", text)]
    overlay_seen = f"loaded relocated overlay: {arena.overlay}" in text
    audio_ready = "[Host] SDL audio ready:" in text
    framebuffer_hashes = set(
        re.findall(r"\[GPU\] framebuffer .*? hash=(0x[0-9A-Fa-f]+)", text)
    )
    overrides_match = re.search(r"loose-file overrides=(\d+)", text)
    source_overrides = int(overrides_match.group(1)) if overrides_match else 0

    if passed and not overlay_seen:
        passed = False
        reason = f"target overlay {arena.overlay} was not loaded"
    if passed and not audio_ready:
        passed = False
        reason = "SDL audio did not initialize"
    if passed and len(heartbeats) < 2:
        passed = False
        reason = "insufficient gameplay heartbeats"
    if passed and len(framebuffer_hashes) < 2:
        passed = False
        reason = "framebuffer did not show multiple distinct rendered states"

    ended = time.time()
    gameplay_seconds = 0.0 if gameplay_started is None else ended - gameplay_started
    return RunResult(
        arena=arena.key,
        mode=mode,
        cycle=cycle,
        passed=passed,
        reason=reason,
        wall_seconds=round(ended - started, 3),
        gameplay_seconds=round(gameplay_seconds, 3),
        heartbeats=len(heartbeats),
        last_gameplay_tick=heartbeats[-1] if heartbeats else 0,
        overlay_seen=overlay_seen,
        audio_ready=audio_ready,
        framebuffer_changes=len(framebuffer_hashes),
        source_overrides=source_overrides,
        stdout_log=str(stdout_path),
        stderr_log=str(stderr_path),
    )


def main() -> int:
    args = parse_args()
    exe = args.exe.resolve()
    if not exe.is_file():
        raise SystemExit(f"Vigilante8PC executable not found: {exe}")
    cue = args.cue.resolve()
    if not cue.is_file():
        raise SystemExit(f"Vigilante 8 CUE not found: {cue}")
    if args.seconds <= 0 or args.entry_timeout <= 0 or args.heartbeat_timeout <= 0:
        raise SystemExit("durations and timeouts must be positive")
    if args.cycles <= 0:
        raise SystemExit("cycles must be positive")

    arenas = selected_arenas(args.maps)
    output = (
        args.output.resolve()
        if args.output
        else DEFAULT_OUTPUT / datetime.now().strftime("%Y%m%d-%H%M%S")
    )
    output.mkdir(parents=True, exist_ok=True)

    results: list[RunResult] = []
    for cycle in range(1, args.cycles + 1):
        for arena in arenas:
            print(
                f"[soak] cycle={cycle} mode={args.mode} arena={arena.key} "
                f"duration={args.seconds:.1f}s",
                flush=True,
            )
            result = run_one(
                exe,
                cue,
                output,
                arena,
                args.mode,
                cycle,
                args.seconds,
                args.entry_timeout,
                args.heartbeat_timeout,
            )
            results.append(result)
            status = "PASS" if result.passed else "FAIL"
            print(
                f"[{status}] {arena.key}: {result.reason}; "
                f"gameplay={result.gameplay_seconds:.1f}s tick={result.last_gameplay_tick}",
                flush=True,
            )
            (output / "summary.json").write_text(
                json.dumps([asdict(item) for item in results], indent=2),
                encoding="utf-8",
            )
            if not result.passed and not args.continue_on_failure:
                return 1

    passed = sum(result.passed for result in results)
    print(f"[soak] complete: {passed}/{len(results)} passed; output={output}")
    return 0 if passed == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
