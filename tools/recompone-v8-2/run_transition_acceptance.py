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
    / "native_arcade_defeat_quit_stock.txt"
)

GUESTS = (
    ("chassey_blue", "guest.v8.chassey_blue", 64, "CHASSEY"),
    ("slick_clyde", "guest.v8.slick_clyde", 65, "CLYDE"),
    ("sheila", "guest.v8.sheila", 66, "SHEILA"),
    ("john_torque", "guest.v8.john_torque", 67, "TORQUE"),
    ("dave", "guest.v8.dave", 68, "CULTSMEN"),
    ("convoy", "guest.v8.convoy", 69, "CONVOY"),
    ("loki", "guest.v8.loki", 70, "BOBO"),
    ("houston_3", "guest.v8.houston_3", 71, "HOUSTON"),
    ("boogie", "guest.v8.boogie", 72, "BOOGIE"),
    ("beezwax", "guest.v8.beezwax", 73, "GARBAGE"),
    ("molo", "guest.v8.molo", 74, "MOLO"),
    ("sid_burn", "guest.v8.sid_burn", 75, "DUSTY"),
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
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "5300",
            "RECOMPONE_V82_UNLOCK_ROSTER": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_UNTHROTTLED": "1",
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
            "[Input] deterministic replay completed at poll 5300" in combined,
        "no_fatal":
            "[Fatal]" not in combined and "Unhandled exception" not in combined,
    }
    if stable_id is not None:
        checks["guest_selected"] = (
            f"selected guest type={expected_type}" in combined
        )
        checks["guest_created"] = (
            f"created {stable_id} identity={expected_type}" in combined
        )
        checks["guest_result_proxy"] = (
            f"imported type={expected_type} resolved native XA stem=" in combined
        )
        checks["guest_result_xa_opened"] = (
            f"start 'SHARED/{expected_xa}.XA'" in combined
        )

    # These are automatically requested by native stage signaling. The
    # acceptance result is textual and headless, so do not retain redundant
    # framebuffer dumps for thirteen otherwise identical transition paths.
    for capture in case_dir.glob("*.ppm"):
        capture.unlink()

    return Result(
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


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--workers", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=180.0)
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

    cases = [("stock", None, None, None), *GUESTS]
    results: list[Result] = []
    with concurrent.futures.ThreadPoolExecutor(
        max_workers=max(1, min(args.workers, len(cases)))
    ) as executor:
        futures = {
            executor.submit(
                run_case,
                exe,
                loose,
                output,
                case,
                stable_id,
                expected_type,
                expected_xa,
                args.timeout,
            ): case
            for case, stable_id, expected_type, expected_xa in cases
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
        "executable_sha256":
            hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
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
