#!/usr/bin/env python3
"""Deterministic multi-map and split-screen soak runner for Vigilante8PC."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
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
    "[v8animationstall]": "animation state-machine stall",
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
    match_mode: int | None
    overlay_seen: bool
    audio_ready: bool
    framebuffer_probes: int
    framebuffer_changes: int
    player2_seen: bool
    player2_position_changes: int
    gameplay_capture: str | None
    final_capture: str | None
    split_left_nonzero: int
    split_right_nonzero: int
    split_halves_distinct: bool
    clean_match_exit: bool
    result_screen_seen: bool
    clean_match_relaunch: bool
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
    parser.add_argument(
        "--clean-exit",
        action="store_true",
        help="Quit through the in-game pause confirmation after the soak interval.",
    )
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


def gameplay_script(mode: str, seconds: float, clean_exit: bool) -> list[str]:
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
    if clean_exit:
        lines.extend(
            [
                "[soak_teardown]",
                "1+2=P1:START",
                "",
                "[pause_1]",
                "20+2=P1:RIGHT",
                "40+2=P1:CROSS",
                "",
                "[pause_confirm_1]",
                "10+2=P1:LEFT",
                "30+2=P1:CROSS",
                "",
                "[pause_quit]",
                "",
            ]
        )
    elif mode == "arcade":
        lines.extend(
            [
                "[result_screen]",
                "320+2=P1:CIRCLE",
                "",
            ]
        )
    return lines


def build_fixture(arena: Arena, mode: str, seconds: float, clean_exit: bool) -> str:
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
    lines.extend(gameplay_script(mode, seconds, clean_exit))
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


def preserve_gameplay_capture(
    directory: Path, output: Path, stem: str, started_at: float
) -> Path | None:
    source = directory / "recompone_capture_gameplay.ppm"
    try:
        if source.stat().st_mtime < started_at - 1.0:
            return None
    except FileNotFoundError:
        return None
    target = output / f"{stem}.gameplay.ppm"
    shutil.copy2(source, target)
    return target


def preserve_final_capture(
    directory: Path, output: Path, stem: str, started_at: float
) -> Path | None:
    source = directory / "recompone_vram_latest.ppm"
    try:
        if source.stat().st_mtime < started_at - 1.0:
            return None
    except FileNotFoundError:
        return None
    target = output / f"{stem}.final.ppm"
    shutil.copy2(source, target)
    return target


def analyze_split_capture(path: Path | None) -> tuple[int, int, bool]:
    if path is None:
        return (0, 0, False)
    try:
        magic, dimensions, maximum, pixels = path.read_bytes().split(b"\n", 3)
        width, height = (int(value) for value in dimensions.split())
        if magic != b"P6" or maximum != b"255" or len(pixels) < width * height * 3:
            return (0, 0, False)
    except (OSError, ValueError):
        return (0, 0, False)

    row_bytes = width * 3
    middle = (width // 2) * 3
    left = bytearray()
    right = bytearray()
    for row_index in range(height):
        row = pixels[row_index * row_bytes : (row_index + 1) * row_bytes]
        left.extend(row[:middle])
        right.extend(row[middle:])

    def nonzero_count(data: bytes) -> int:
        return sum(
            1
            for offset in range(0, len(data), 3)
            if data[offset] or data[offset + 1] or data[offset + 2]
        )

    left_nonzero = nonzero_count(left)
    right_nonzero = nonzero_count(right)
    return (left_nonzero, right_nonzero, hash(bytes(left)) != hash(bytes(right)))


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
    clean_exit: bool,
) -> RunResult:
    stem = f"{cycle:02d}_{mode}_{arena.key}"
    fixture_path = output / f"{stem}.input.txt"
    stdout_path = output / f"{stem}.stdout.log"
    stderr_path = output / f"{stem}.stderr.log"
    fixture_path.write_text(
        build_fixture(arena, mode, seconds, clean_exit), encoding="utf-8"
    )

    env = os.environ.copy()
    env["RECOMPONE_INPUT_FILE"] = str(fixture_path.resolve())
    env["RECOMPONE_DISABLE_LIVE_INPUT"] = "1"
    env["RECOMPONE_FORCE_PAD2_CONNECTED"] = "1" if mode != "arcade" else "0"
    env["RECOMPONE_WINDOW_VISIBLE"] = "0"
    env["RECOMPONE_V8_GAME_VOLUME"] = "0"
    env["RECOMPONE_SOAK_HEARTBEAT_TICKS"] = "180"
    env["RECOMPONE_DISPLAY_PROBE_INTERVAL"] = "60"
    env["RECOMPONE_TARGET_LOCATION"] = arena.display
    if mode != "arcade":
        env["RECOMPONE_TARGET_TWO_PLAYER_MODE"] = (
            "COOPERATIVE" if mode == "coop" else "VERSUS"
        )
    env["RECOMPONE_TRACE_LEVEL_LOAD"] = "1"
    env["RECOMPONE_TRACE_RESULTS"] = "1"
    env["RECOMPONE_GAMEPLAY_CAPTURE_DELAY_POLLS"] = "300"
    if clean_exit:
        env["RECOMPONE_SOAK_TEARDOWN_TICKS"] = str(max(180, round(seconds * 60)))

    creationflags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    started = time.time()
    gameplay_started: float | None = None
    last_framebuffer_at: float | None = None
    heartbeat_count = 0
    framebuffer_count = 0
    result_heartbeat_count: int | None = None
    result_started: float | None = None
    reason = ""
    passed = False
    gameplay_capture: Path | None = None
    final_capture: Path | None = None

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
                    last_framebuffer_at = now
                    print(f"[{stem}] gameplay reached", flush=True)

                heartbeat_matches = list(
                    re.finditer(r"\[Soak\] gameplay tick=(\d+)", text)
                )
                if len(heartbeat_matches) > heartbeat_count:
                    heartbeat_count = len(heartbeat_matches)

                framebuffer_matches = re.findall(r"\[GPU\] framebuffer ", text)
                if len(framebuffer_matches) > framebuffer_count:
                    framebuffer_count = len(framebuffer_matches)
                    last_framebuffer_at = now

                if (
                    result_started is None
                    and "[Input] stage 'result_screen'" in text
                ):
                    result_started = now
                    result_heartbeat_count = heartbeat_count
                    print(f"[{stem}] result screen reached", flush=True)

                overlay_load_count = text.count(
                    f"loaded relocated overlay: {arena.overlay}"
                )
                clean_match_relaunch = (
                    result_started is not None
                    and overlay_load_count >= 2
                    and result_heartbeat_count is not None
                    and heartbeat_count > result_heartbeat_count
                )
                if clean_match_relaunch:
                    passed = True
                    reason = "clean match completion, result dismissal, and gameplay relaunch"
                    break

                clean_match_exit = "[Input] stage 'pause_quit'" in text
                if clean_exit and clean_match_exit:
                    passed = True
                    reason = "completed requested gameplay duration and clean in-game exit"
                    break

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
                    if not clean_exit and gameplay_elapsed >= seconds:
                        passed = True
                        reason = "completed requested gameplay duration"
                        break
                    if clean_exit and gameplay_elapsed >= seconds + 30.0:
                        reason = "in-game exit timeout"
                        break
                    if (
                        last_framebuffer_at is not None
                        and now - last_framebuffer_at > heartbeat_timeout
                    ):
                        reason = "render heartbeat timeout"
                        break
                    if result_started is not None and now - result_started > 45.0:
                        reason = "result teardown or gameplay relaunch timeout"
                        break

                time.sleep(0.5)
        finally:
            stop_process(process)
            gameplay_capture = preserve_gameplay_capture(
                exe.parent, output, stem, started
            )
            final_capture = preserve_final_capture(exe.parent, output, stem, started)
            cleanup_runtime_artifacts(exe.parent, started)

    text = combined_text(stdout_path, stderr_path)
    heartbeats = [int(value) for value in re.findall(r"\[Soak\] gameplay tick=(\d+)", text)]
    match_modes = [int(value) for value in re.findall(r"\[Soak\].*? match_mode=(\d+)", text)]
    overlay_seen = f"loaded relocated overlay: {arena.overlay}" in text
    audio_ready = "[Host] SDL audio ready:" in text
    framebuffer_hashes = set(
        re.findall(r"\[GPU\] framebuffer .*? hash=(0x[0-9A-Fa-f]+)", text)
    )
    player2_samples = re.findall(
        r"player2=(0x[0-9A-Fa-f]+)(?: pos2=(\([^\r\n]+?\)))?", text
    )
    player2_seen = any(int(address, 16) != 0 for address, _ in player2_samples)
    player2_positions = {
        position
        for address, position in player2_samples
        if int(address, 16) != 0 and position
    }
    split_left_nonzero, split_right_nonzero, split_halves_distinct = (
        analyze_split_capture(gameplay_capture)
    )
    clean_match_exit = "[Input] stage 'pause_quit'" in text
    result_screen_seen = "[Input] stage 'result_screen'" in text
    result_offset = text.find("[Input] stage 'result_screen'")
    post_result_text = text[result_offset:] if result_offset >= 0 else ""
    clean_match_relaunch = (
        result_screen_seen
        and text.count(f"loaded relocated overlay: {arena.overlay}") >= 2
        and bool(re.search(r"\[Soak\] gameplay tick=\d+", post_result_text))
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
    if passed and mode != "arcade" and not player2_seen:
        passed = False
        reason = "second player object was not present in gameplay"
    if passed and mode != "arcade" and len(player2_positions) < 2:
        passed = False
        reason = "second player position did not change"
    if passed and mode != "arcade" and (
        split_left_nonzero < 1000 or split_right_nonzero < 1000
    ):
        passed = False
        reason = "split-screen capture did not render both screen halves"
    if passed and mode != "arcade" and not split_halves_distinct:
        passed = False
        reason = "split-screen capture halves were not distinct"
    expected_match_mode = {"coop": 4, "versus": 3}.get(mode)
    if passed and expected_match_mode is not None and (
        not match_modes or match_modes[-1] != expected_match_mode
    ):
        passed = False
        actual = match_modes[-1] if match_modes else "missing"
        reason = f"expected match mode {expected_match_mode}, observed {actual}"
    if passed and clean_exit and not clean_match_exit:
        passed = False
        reason = "in-game exit was not observed"

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
        match_mode=match_modes[-1] if match_modes else None,
        overlay_seen=overlay_seen,
        audio_ready=audio_ready,
        framebuffer_probes=framebuffer_count,
        framebuffer_changes=len(framebuffer_hashes),
        player2_seen=player2_seen,
        player2_position_changes=len(player2_positions),
        gameplay_capture=str(gameplay_capture) if gameplay_capture else None,
        final_capture=str(final_capture) if final_capture else None,
        split_left_nonzero=split_left_nonzero,
        split_right_nonzero=split_right_nonzero,
        split_halves_distinct=split_halves_distinct,
        clean_match_exit=clean_match_exit,
        result_screen_seen=result_screen_seen,
        clean_match_relaunch=clean_match_relaunch,
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
                args.clean_exit,
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
