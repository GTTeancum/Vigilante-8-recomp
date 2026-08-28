#!/usr/bin/env python3
"""Headless defeat/result/quit/menu-return acceptance for V8:2.

The sweep deliberately includes every imported V8 vehicle because the native
result builder owns a fixed 18-entry V8:2 character/audio table. A guest that
survives gameplay can still fail only when that table is consumed after defeat.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import re
import subprocess
import time
from dataclasses import asdict, dataclass
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
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "renderer-rewrite-transition-acceptance"
FIXTURE = (
    REPO
    / "tools"
    / "recompone-v8-2"
    / "input-scripts"
    / "native_arcade_defeat_voice_full.txt"
)

GUESTS = (
    ("chassey_blue", "guest.v8.chassey_blue", 64, "V8VOICE/D00"),
    ("slick_clyde", "guest.v8.slick_clyde", 65, "V8VOICE/D01"),
    ("sheila", "guest.v8.sheila", 66, "V8VOICE/D02"),
    ("john_torque", "guest.v8.john_torque", 67, "V8VOICE/D03"),
    ("dave", "guest.v8.dave", 68, "V8VOICE/D04"),
    ("convoy", "guest.v8.convoy", 69, "V8VOICE/D05"),
    ("loki", "guest.v8.loki", 70, "V8VOICE/D06"),
    ("houston_3", "guest.v8.houston_3", 71, "V8VOICE/D07"),
    ("boogie", "guest.v8.boogie", 72, "V8VOICE/D08"),
    ("beezwax", "guest.v8.beezwax", 73, "V8VOICE/D09"),
    ("molo", "guest.v8.molo", 74, "V8VOICE/D10"),
    ("sid_burn", "guest.v8.sid_burn", 75, "V8VOICE/D11"),
    ("y_the_alien", "guest.v8.y_the_alien", 76, "V8VOICE/D12"),
)


@dataclass(frozen=True)
class Result:
    case: str
    stable_id: str | None
    expected_type: int | None
    expected_xa: str | None
    exit_code: int
    elapsed_seconds: float
    passed: bool
    checks: dict[str, bool]
    stdout: str
    stderr: str


def run_case(
    exe: Path,
    loose: Path,
    output: Path,
    executable_sha256: str,
    case: str,
    stable_id: str | None,
    expected_type: int | None,
    expected_xa: str | None,
    timeout: float,
) -> Result:
    case_dir = output / case
    case_dir.mkdir(parents=True, exist_ok=True)
    stdout_path = case_dir / "stdout.log"
    stderr_path = case_dir / "stderr.log"
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_INPUT_FILE": str(FIXTURE),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_HEADLESS": "1",
            "RECOMPONE_GPU_HLE": "1",
            "RECOMPONE_V82_TEST_DEFEAT_FRAME": "120",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "6000",
            "RECOMPONE_V82_UNLOCK_ROSTER": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_UNTHROTTLED": "0",
            "RECOMPONE_CAPTURE_DIR": str(case_dir),
        }
    )
    for key in (
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE",
        "RECOMPONE_PRESENTATION_CAPTURE",
        "RECOMPONE_PRESENTATION_CAPTURE_FRAMES",
        "RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES",
        "RECOMPONE_V82_PLAYER_TYPE",
    ):
        env.pop(key, None)

    command = [str(exe), "--loose", str(loose)]
    if stable_id is not None:
        command += ["--guest-vehicle", stable_id]

    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    started = time.monotonic()
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        try:
            completed = subprocess.run(
                command,
                cwd=case_dir,
                env=env,
                stdout=stdout,
                stderr=stderr,
                timeout=timeout,
                creationflags=flags,
                check=False,
            )
            exit_code = completed.returncode
        except subprocess.TimeoutExpired:
            exit_code = 124
    elapsed = time.monotonic() - started
    stdout_text = stdout_path.read_text(encoding="utf-8", errors="replace")
    stderr_text = stderr_path.read_text(encoding="utf-8", errors="replace")
    combined = stdout_text + "\n" + stderr_text

    checks = {
        "clean_exit": exit_code == 0,
        "native_lethal_damage":
            "[V82DefeatRegression] native lethal damage completed" in combined,
        "defeat_stage": "[Input] stage 'defeated'" in combined,
        "result_built": "sprintf caller=0x80013268" in combined,
        "returned_to_shell":
            "loaded relocated overlay: SHELL_SHELL" in combined,
        "menu_music_after_return":
            "[CDDA] loose track=2 source=music/track02.ogg streaming" in combined,
        "replay_completed":
            "[Input] deterministic replay completed at poll 6000" in combined,
        "no_fatal":
            "[Fatal]" not in combined and "Unhandled exception" not in combined,
    }
    if stable_id is not None:
        xa_path = loose / "SHARED" / f"{expected_xa}.XA"
        xa_bytes = xa_path.stat().st_size
        if xa_bytes % (2336 * 8):
            raise ValueError(
                f"{xa_path} is not an eight-sector-cadence XA file"
            )
        expected_audio_sectors = xa_bytes // (2336 * 8)
        expected_duration = expected_audio_sectors * 4032 / 37800
        stream_end = re.search(
            rf"\[CdStream\] end 'SHARED/{re.escape(expected_xa)}\.XA' "
            rf"reason=file-end frames=0 xa=(\d+) last=-1 elapsed=([\d.]+)s",
            combined,
        )
        checks["guest_selected"] = (
            f"selected guest type={expected_type}" in combined
        )
        checks["guest_created"] = (
            f"created {stable_id} identity={expected_type}" in combined
        )
        checks["guest_result_voice"] = (
            f"outcome=defeat channel={expected_type - 64} "
            f"stem={expected_xa.replace('/', chr(92))}" in combined
        )
        checks["guest_result_native_path"] = (
            f"wrote original V8 XA path="
            f"'Shared{chr(92)}{expected_xa.replace('/', chr(92))}.xa'"
            in combined
        )
        # Imported paths are written directly into the retail caller's result
        # buffer, so the skipped sprintf has no trace line of its own.
        checks["result_built"] = checks["guest_result_native_path"]
        checks["guest_result_xa_opened"] = (
            f"start 'SHARED/{expected_xa}.XA'" in combined
        )
        checks["guest_result_full_voice"] = (
            stream_end is not None
            and int(stream_end.group(1)) == expected_audio_sectors
            and float(stream_end.group(2)) >= expected_duration - 0.15
        )

    # These are automatically requested by native stage signaling. The
    # acceptance result is textual and headless, so do not retain redundant
    # framebuffer dumps for thirteen otherwise identical transition paths.
    for capture in case_dir.glob("*.ppm"):
        capture.unlink()

    result = Result(
        case=case,
        stable_id=stable_id,
        expected_type=expected_type,
        expected_xa=expected_xa,
        exit_code=exit_code,
        elapsed_seconds=round(elapsed, 3),
        passed=all(checks.values()),
        checks=checks,
        stdout=str(stdout_path),
        stderr=str(stderr_path),
    )
    case_report = {
        "schema": 1,
        "executable_sha256": executable_sha256,
        "loose_root": str(loose),
        "result": asdict(result),
    }
    (case_dir / "result.json").write_text(
        json.dumps(case_report, indent=2) + "\n",
        encoding="utf-8",
    )
    return result


def load_checkpoint(
    output: Path,
    executable_sha256: str,
    loose: Path,
    case: str,
    stable_id: str | None,
    expected_type: int | None,
    expected_xa: str | None,
) -> Result | None:
    path = output / case / "result.json"
    if not path.is_file():
        return None
    try:
        report = json.loads(path.read_text(encoding="utf-8"))
        result = Result(**report["result"])
    except (KeyError, TypeError, ValueError, json.JSONDecodeError):
        return None
    if (
        report.get("schema") != 1
        or report.get("executable_sha256") != executable_sha256
        or report.get("loose_root") != str(loose)
        or result.case != case
        or result.stable_id != stable_id
        or result.expected_type != expected_type
        or result.expected_xa != expected_xa
        or not result.passed
    ):
        return None
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--workers", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument(
        "--resume",
        action="store_true",
        help=(
            "reuse passed per-case checkpoints only when the staged "
            "executable hash and loose root match exactly"
        ),
    )
    parser.add_argument(
        "--case",
        action="append",
        dest="selected_cases",
        help="run only the named case (repeatable)",
    )
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    output = args.output.resolve()
    if not exe.is_file():
        raise FileNotFoundError(exe)
    if not (loose / "SLUS_008.68").is_file():
        raise FileNotFoundError(loose / "SLUS_008.68")
    if not FIXTURE.is_file():
        raise FileNotFoundError(FIXTURE)
    output.mkdir(parents=True, exist_ok=True)
    executable_sha256 = hashlib.sha256(exe.read_bytes()).hexdigest().upper()

    cases = [("stock", None, None, None), *GUESTS]
    if args.selected_cases:
        requested = set(args.selected_cases)
        known = {case[0] for case in cases}
        unknown = sorted(requested - known)
        if unknown:
            raise ValueError(f"unknown transition case(s): {', '.join(unknown)}")
        cases = [case for case in cases if case[0] in requested]
    results: list[Result] = []
    pending_cases = cases
    if args.resume:
        pending_cases = []
        for case, stable_id, expected_type, expected_xa in cases:
            checkpoint = load_checkpoint(
                output,
                executable_sha256,
                loose,
                case,
                stable_id,
                expected_type,
                expected_xa,
            )
            if checkpoint is None:
                pending_cases.append(
                    (case, stable_id, expected_type, expected_xa)
                )
                continue
            results.append(checkpoint)
            print(
                f"[TransitionAcceptance] case={case} "
                "pass=True source=checkpoint",
                flush=True,
            )
    with concurrent.futures.ThreadPoolExecutor(
        max_workers=max(1, min(args.workers, max(1, len(pending_cases))))
    ) as executor:
        futures = {
            executor.submit(
                run_case,
                exe,
                loose,
                output,
                executable_sha256,
                case,
                stable_id,
                expected_type,
                expected_xa,
                args.timeout,
            ): case
            for case, stable_id, expected_type, expected_xa in pending_cases
        }
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            print(
                f"[TransitionAcceptance] case={result.case} "
                f"pass={result.passed} exit={result.exit_code} "
                f"elapsed={result.elapsed_seconds:.3f}s",
                flush=True,
            )

    results.sort(key=lambda item: [case[0] for case in cases].index(item.case))
    report = {
        "schema": 1,
        "passed": all(result.passed for result in results),
        "cases_passed": sum(result.passed for result in results),
        "cases_total": len(results),
        "executable": str(exe),
        "executable_sha256": executable_sha256,
        "loose_root": str(loose),
        "results": [asdict(result) for result in results],
    }
    report_path = output / "acceptance.json"
    report_path.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[TransitionAcceptance] pass={report['passed']} "
        f"cases={report['cases_passed']}/{report['cases_total']} "
        f"report={report_path}"
    )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
