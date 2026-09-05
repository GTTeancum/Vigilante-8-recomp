#!/usr/bin/env python3
"""Hidden/silent two-match V8:2 smoke matrix with HD-texture proof."""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import subprocess
import time
from dataclasses import asdict, dataclass
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-multi-map-smoke"

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

GUESTS = (
    "guest.v8.chassey_blue",
    "guest.v8.slick_clyde",
    "guest.v8.sheila",
    "guest.v8.john_torque",
    "guest.v8.dave",
    "guest.v8.convoy",
    "guest.v8.loki",
    "guest.v8.houston_3",
    "guest.v8.boogie",
    "guest.v8.beezwax",
    "guest.v8.molo",
    "guest.v8.sid_burn",
    "guest.v8.y_the_alien",
)


@dataclass(frozen=True)
class Case:
    name: str
    group: str
    map_a: int
    map_b: int
    type_a: int | None = None
    type_b: int | None = None
    guest: str | None = None


@dataclass(frozen=True)
class Result:
    case: str
    group: str
    exit_code: int
    elapsed_seconds: float
    passed: bool
    checks: dict[str, bool]
    stdout: str
    stderr: str
    runtime_log: str


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def navigation(stage: str, start_slot: int, target_slot: int) -> list[str]:
    count = len(MAPS)
    right = (target_slot - start_slot) % count
    left = (start_slot - target_slot) % count
    direction, steps = ("RIGHT", right) if right <= left else ("LEFT", left)
    lines = [f"[{stage}]"]
    # Re-entered selectors may still be decoding their initial preview at 350
    # polls even though the stage itself is live. Begin after the same 900-poll
    # settle used by the slow carousel group so the first edge is never lost.
    poll = 900
    slot = start_slot
    for _ in range(steps):
        next_slot = (slot + (1 if direction == "RIGHT" else -1)) % count
        lines.append(f"{poll}+2={direction}")
        # Several retail previews continue consuming input for more than 240
        # polls under concurrent hidden-GL load. A uniform 480-poll settle for
        # ordinary slots avoids papering over individual slow maps; the known
        # late carousel group retains its longer 900-poll native settle.
        poll += 900 if max(slot, next_slot) >= 15 else 480
        slot = next_slot
    lines.append(f"{poll + 350}+3=CROSS")
    return lines


def fixture_text(case: Case) -> str:
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
        "# Returned shell omits the boot FMV; replay only its menu path.",
        "[after:shell_transition@2]",
        "540+3=START",
        "660+2=SELECT",
        "720+3=CROSS",
        "",
        "[choose_location_1]",
        "1250+3=CROSS",
        "",
        "[choose_location_2]",
        "1250+3=CROSS",
        "",
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
        "700+2=SELECT",
        "1000+30=CROSS",
        "1400+12=UP",
        "1600+12=DOWN",
        # Some guest defeat voice clips keep the retail result screen busy past
        # the first confirm.  The stage latch cancels this retry as soon as the
        # shell transition begins, so it is safe for both short and long clips.
        "1800+30=CROSS",
    ]
    return "\n".join(lines) + "\n"


def cases() -> list[Case]:
    result: list[Case] = []
    for index in range(31):
        map_a = index % len(MAPS)
        map_b = (map_a + 9) % len(MAPS)
        if index < 18:
            result.append(Case(
                f"character_{index:02d}_native_{index:02d}",
                "every_character", map_a, map_b, index, index,
            ))
        else:
            guest = GUESTS[index - 18]
            result.append(Case(
                f"character_{index:02d}_{guest.rsplit('.', 1)[-1]}",
                "every_character", map_a, map_b, guest=guest,
            ))
    for index, pair in enumerate(((0, 1), (2, 3), (4, 5), (6, 7), (8, 9))):
        result.append(Case(
            f"same_level_different_characters_{index + 1}",
            "same_level_different_characters", 0, 0, pair[0], pair[1],
        ))
    map_pairs = ((0, 18), (3, 11), (7, 2), (14, 5), (9, 16))
    type_pairs = ((10, 11), (12, 13), (14, 15), (16, 17), (1, 8))
    for index, (map_pair, type_pair) in enumerate(zip(map_pairs, type_pairs)):
        result.append(Case(
            f"different_level_different_characters_{index + 1}",
            "different_level_different_characters",
            map_pair[0], map_pair[1], type_pair[0], type_pair[1],
        ))
    return result


def location_token(slot: int) -> str:
    return "n64.super_dreamland_64" if slot == 18 else f"retail.{slot}"


def run_case(
    case: Case,
    exe: Path,
    loose: Path,
    output: Path,
    exe_hash: str,
    gameplay_frames: int,
    timeout: float,
    navigation_probe: bool,
) -> Result:
    case_dir = output / case.name
    case_dir.mkdir(parents=True, exist_ok=True)
    fixture = case_dir / "input.txt"
    fixture.write_text(fixture_text(case), encoding="utf-8")
    stdout_path = case_dir / "stdout.log"
    stderr_path = case_dir / "stderr.log"
    runtime_path = case_dir / "runtime.log"
    env = os.environ.copy()
    env.update({
        "RECOMPONE_INPUT_FILE": str(fixture),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_GPU_HLE": "1",
        "RECOMPONE_MUTE": "1",
        "SDL_AUDIODRIVER": "dummy",
        "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_UNTHROTTLED": "0",
        "RECOMPONE_TRACE_INPUT": "1",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_TRACE_LOADING_UI_TEXTURES": "1",
        "RECOMPONE_TRACE_MDEC": "1",
        "RECOMPONE_TRACE_NATIVE_OPTIONS": "1",
        "RECOMPONE_TRACE_V82_ARENA_SELECTOR": "1",
        "RECOMPONE_V82_ARENA_SLOT_SEQUENCE":
            f"{case.map_a},{case.map_b}",
        "RECOMPONE_LOG_PATH": str(runtime_path),
        "RECOMPONE_V82_SOAK": "1",
        "RECOMPONE_V82_UNLOCK_ROSTER": "1",
        "RECOMPONE_V82_SOAK_POWERUPS": "0",
        "RECOMPONE_V82_SOAK_WEAPONS": "0",
        "RECOMPONE_SOAK_TEARDOWN_FRAMES": "0",
        "RECOMPONE_V82_TEST_DEFEAT_FRAME": str(gameplay_frames),
        # Do not stop at the second return's overlay notification: a resident
        # SHELL can enter its native title dispatcher before that notification.
        # Wait until OPTIONS is submitted on the second returned menu so both
        # post-match screens are actually covered by the contract below.
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE": "v82_options",
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE_VISITS": "3",
        "RECOMPONE_PRESENTATION_CAPTURE": "0",
        "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "0",
        "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
        "RECOMPONE_CAPTURE_SELECTOR_GENERATIONS": "0",
        "RECOMPONE_CAPTURE_DIR": str(case_dir),
    })
    for key in (
        "RECOMPONE_HEADLESS",
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS",
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE",
        "RECOMPONE_PRESENTATION_CAPTURE_FRAMES",
        "RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES",
        "RECOMPONE_V82_PLAYER_TYPE",
        "RECOMPONE_V82_PLAYER_TYPE_SEQUENCE",
    ):
        env.pop(key, None)
    if case.guest is None:
        env["RECOMPONE_V82_PLAYER_TYPE_SEQUENCE"] = (
            f"{case.type_a},{case.type_b}"
        )
    if navigation_probe:
        env["RECOMPONE_SCRIPT_EXIT_AFTER_STAGE"] = "choose_player"
        env["RECOMPONE_SCRIPT_EXIT_AFTER_STAGE_VISITS"] = "2"

    command = [str(exe), "--loose", str(loose)]
    if case.guest is not None:
        command += ["--guest-vehicle", case.guest]
    started = time.monotonic()
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        try:
            complete = subprocess.run(
                command,
                cwd=loose,
                env=env,
                stdout=stdout,
                stderr=stderr,
                timeout=timeout,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                check=False,
            )
            exit_code = complete.returncode
        except subprocess.TimeoutExpired:
            exit_code = 124
    elapsed = time.monotonic() - started
    text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in (stdout_path, stderr_path, runtime_path)
        if path.is_file()
    )
    runtime_text = runtime_path.read_text(
        encoding="utf-8", errors="replace"
    ) if runtime_path.is_file() else text
    shell_marker = "[Input] stage 'shell_transition'"
    shell_positions: list[int] = []
    marker_at = runtime_text.find(shell_marker)
    while marker_at >= 0:
        shell_positions.append(marker_at)
        marker_at = runtime_text.find(shell_marker, marker_at + 1)
    title_marker = "[V82NativeTitleEntry]"
    title_positions: list[int] = []
    marker_at = runtime_text.find(title_marker)
    while marker_at >= 0:
        title_positions.append(marker_at)
        marker_at = runtime_text.find(title_marker, marker_at + 1)
    title_shells = [
        runtime_text[start:(
            title_positions[index + 1]
            if index + 1 < len(title_positions)
            else len(runtime_text)
        )]
        for index, start in enumerate(title_positions)
    ]
    returned_shells = title_shells[1:3]
    first_return_audio = returned_shells[0] if returned_shells else ""
    arena_a, terrain_a = MAPS[case.map_a]
    arena_b, terrain_b = MAPS[case.map_b]
    checks = {
        "clean_exit": exit_code == 0,
        "exact_executable": exe_hash in runtime_text.upper(),
        "three_shell_visits":
            runtime_text.count("[Input] stage 'shell_transition'") >= 3,
        "two_gameplay_visits":
            runtime_text.count("[Input] stage 'gameplay'") >= 2,
        "two_native_defeats":
            runtime_text.count(
                "[V82DefeatRegression] native lethal damage completed") >= 2,
        "deterministic_completion":
            "deterministic replay completed after stage 'v82_options' visit 3"
            in runtime_text,
        "first_location": f"[V82Arena] location={location_token(case.map_a)}" in text,
        "second_location": f"[V82Arena] location={location_token(case.map_b)}" in text,
        "second_selector_baseline":
            f"generation=2 retail=18 imported=1 total=19 baseline-slot={case.map_a}"
            in text,
        "arena_slot_sequence": all(
            f"arena-slot-sequence generation={index} index={index - 1} "
            f"slot={slot}" in runtime_text
            for index, slot in enumerate((case.map_a, case.map_b), start=1)
        ),
        "nineteen_hd_loading_cards":
            "[TexturePack] loaded 19 loading card overlays" in text,
        "first_hd_loading_card":
            f"selected loading card overlay arena={arena_a}:" in text,
        "second_hd_loading_card":
            runtime_text.count(
                f"selected loading card overlay arena={arena_b}:")
            >= (2 if case.map_a == case.map_b else 1),
        "first_hd_terrain": f"active terrain atlas={terrain_a} " in text,
        "second_hd_terrain":
            f"active terrain atlas={terrain_b} " in text
            if case.map_a != case.map_b
            else f"active terrain atlas={terrain_a} " in text,
        "terrain_runtime_hits":
            runtime_text.count("[TexturePack] terrain atlas hit key=")
            >= (2 if case.map_a != case.map_b else 1),
        "menu_audio_recovered":
            "[CDDA] play LBA=147300" in first_return_audio
            or "[CDDA] loose track=2 " in first_return_audio,
        "every_post_match_skips_press_start":
            len(returned_shells) == 2
            and all(
                f"entry={entry} mode=5" in segment
                and "[V82NativeTitlePromptRoutine]" not in segment
                and "text='PRESS START'" not in segment
                for entry, segment in enumerate(returned_shells, start=2)
            ),
        "every_post_match_has_main_menu_labels":
            len(returned_shells) == 2
            and all(
                all(
                    f"[V82NativeOptions] text='{label}' caller=0x80104284"
                    in segment
                    for label in ("1 PLAYER", "2 PLAYER", "OPTIONS")
                ) and "[Input] stage 'v82_options'" in segment
                for segment in returned_shells
            ),
        "no_fatal":
            "[Fatal]" not in text
            and "Unhandled exception" not in text
            and "unmapped address" not in text.lower()
            and "out of vram" not in text.lower(),
    }
    if case.guest is None:
        checks["native_type_sequence"] = all(
            f"player-type-sequence match={index} index={index - 1} type={vehicle_type}"
            in text
            for index, vehicle_type in enumerate(
                (case.type_a, case.type_b), start=1
            )
        )
    else:
        checks["guest_created_twice"] = (
            text.count(f"created {case.guest} identity=") >= 2
        )

    for image in case_dir.glob("*.ppm"):
        image.unlink()
    result = Result(
        case.name,
        case.group,
        exit_code,
        round(elapsed, 3),
        all(checks.values()),
        checks,
        str(stdout_path),
        str(stderr_path),
        str(runtime_path),
    )
    report = {
        "schema": 2,
        "executable_sha256": exe_hash,
        "gameplay_frames": gameplay_frames,
        "case_definition": asdict(case),
        "result": asdict(result),
    }
    (case_dir / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    return result


def load_pass(
    case: Case, output: Path, exe_hash: str, gameplay_frames: int
) -> Result | None:
    path = output / case.name / "acceptance.json"
    if not path.is_file():
        return None
    try:
        report = json.loads(path.read_text(encoding="utf-8"))
        result = Result(**report["result"])
    except (KeyError, TypeError, ValueError, json.JSONDecodeError):
        return None
    if (
        report.get("schema") != 2
        or report.get("executable_sha256") != exe_hash
        or report.get("gameplay_frames") != gameplay_frames
        or report.get("case_definition") != asdict(case)
        or not result.passed
    ):
        return None
    return result


def write_summary(
    output: Path,
    exe_hash: str,
    gameplay_frames: int,
    selected: list[Case],
    results: dict[str, Result],
) -> None:
    ordered = [results[case.name] for case in selected if case.name in results]
    report = {
        "schema": 2,
        "executable_sha256": exe_hash,
        "gameplay_frames_per_match": gameplay_frames,
        "matches_requested": len(selected) * 2,
        "cases_requested": len(selected),
        "cases_finished": len(ordered),
        "cases_passed": sum(result.passed for result in ordered),
        "passed": len(ordered) == len(selected) and all(
            result.passed for result in ordered
        ),
        "results": [asdict(result) for result in ordered],
    }
    (output / "summary.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--workers", type=int, default=1)
    parser.add_argument("--gameplay-frames", type=int, default=1440)
    parser.add_argument("--timeout", type=float, default=1200.0)
    parser.add_argument("--pilot", action="store_true")
    parser.add_argument("--pilot-map-a", type=int, default=0)
    parser.add_argument("--pilot-map-b", type=int, default=1)
    parser.add_argument("--reuse-passing", action="store_true")
    parser.add_argument("--navigation-probe", action="store_true")
    parser.add_argument(
        "--only", action="append", default=[],
        help="Run only exact case names (repeatable).",
    )
    args = parser.parse_args()
    exe, loose, output = (
        args.exe.resolve(), args.loose.resolve(), args.output.resolve()
    )
    output.mkdir(parents=True, exist_ok=True)
    exe_hash = sha256(exe)
    selected = cases()
    if args.only:
        requested = set(args.only)
        selected = [case for case in selected if case.name in requested]
        missing = requested - {case.name for case in selected}
        if missing:
            parser.error("unknown case name(s): " + ", ".join(sorted(missing)))
    if args.pilot:
        for slot in (args.pilot_map_a, args.pilot_map_b):
            if slot < 0 or slot >= len(MAPS):
                parser.error(f"pilot map slot must be 0..{len(MAPS) - 1}")
        selected = [
            Case(
                "pilot_target", "pilot", args.pilot_map_a,
                args.pilot_map_b, 0, 1,
            ),
            Case(
                "pilot_parallel_load", "pilot",
                (args.pilot_map_a + 1) % len(MAPS),
                (args.pilot_map_b + 1) % len(MAPS), 2, 3,
            ),
        ]
    results: dict[str, Result] = {}
    pending: list[Case] = []
    for case in selected:
        cached = (
            load_pass(case, output, exe_hash, args.gameplay_frames)
            if args.reuse_passing else None
        )
        if cached is None:
            pending.append(case)
        else:
            results[case.name] = cached
            print(f"[MultiMapSmoke] REUSE PASS {case.name}", flush=True)
    write_summary(output, exe_hash, args.gameplay_frames, selected, results)
    with concurrent.futures.ThreadPoolExecutor(
        max_workers=max(1, args.workers)
    ) as pool:
        future_cases = {
            pool.submit(
                run_case, case, exe, loose, output, exe_hash,
                args.gameplay_frames, args.timeout, args.navigation_probe,
            ): case
            for case in pending
        }
        for future in concurrent.futures.as_completed(future_cases):
            case = future_cases[future]
            try:
                result = future.result()
            except Exception as exc:
                print(f"[MultiMapSmoke] ERROR {case.name}: {exc}", flush=True)
                continue
            results[case.name] = result
            state = "PASS" if result.passed else "FAIL"
            failed = ",".join(
                name for name, passed in result.checks.items() if not passed
            )
            print(
                f"[MultiMapSmoke] {state} {case.name} "
                f"elapsed={result.elapsed_seconds:.1f}s"
                + (f" failed={failed}" if failed else ""),
                flush=True,
            )
            write_summary(output, exe_hash, args.gameplay_frames, selected, results)
    write_summary(output, exe_hash, args.gameplay_frames, selected, results)
    return 0 if len(results) == len(selected) and all(
        result.passed for result in results.values()
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
