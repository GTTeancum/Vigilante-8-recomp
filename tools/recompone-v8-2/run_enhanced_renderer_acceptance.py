#!/usr/bin/env python3
"""Hidden GL telemetry matrix for the V8:2 Enhanced renderer.

Long 2,700-frame soaks establish gameplay/event/attachment stability. This
matrix is the complementary renderer gate: it samples every arena with every
stock character represented once, then every imported V8 vehicle, while the
Enhanced backend emits geometry-provenance, fallback, glass, and fatal-state
telemetry.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path

from analyze_enhanced_captures import analyze_renderer_log


REPO = Path(__file__).resolve().parents[2]
SOAK = Path(__file__).with_name("run_reference_soak.py")
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
DEFAULT_OUTPUT = (
    REPO / "artifacts" / "renderer-rewrite-final-enhanced-telemetry"
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
)


@dataclass(frozen=True)
class MatrixCase:
    label: str
    map_slot: int
    character_slot: int
    guest_vehicle: str | None


@dataclass(frozen=True)
class CaseResult:
    label: str
    map_slot: int
    character_slot: int
    guest_vehicle: str | None
    exit_code: int
    elapsed_seconds: float
    soak_passed: bool
    renderer_passed: bool
    maximum_visible_world_fallback_percent: float | None
    maximum_glass_primitives: int
    stderr_log: str | None
    passed: bool


def build_cases() -> list[MatrixCase]:
    cases = [
        MatrixCase(
            label=f"stock_map_{slot:02d}_character_{slot:02d}",
            map_slot=slot,
            character_slot=slot,
            guest_vehicle=None,
        )
        for slot in range(18)
    ]
    cases += [
        MatrixCase(
            label=f"guest_{stable_id.removeprefix('guest.v8.')}",
            map_slot=index,
            character_slot=0,
            guest_vehicle=stable_id,
        )
        for index, stable_id in enumerate(GUESTS)
    ]
    return cases


def run_case(
    case: MatrixCase,
    exe: Path,
    loose: Path,
    output: Path,
    frames: int,
    timeout: float,
) -> CaseResult:
    case_dir = output / case.label
    case_dir.mkdir(parents=True, exist_ok=True)
    command = [
        sys.executable,
        str(SOAK),
        "--exe",
        str(exe),
        "--loose-root",
        str(loose),
        "--output",
        str(case_dir),
        "--maps",
        str(case.map_slot),
        "--characters",
        str(case.character_slot),
        "--frames",
        str(frames),
        "--cycles",
        "1",
        "--coverage-profile",
        "combined",
        "--entry-timeout",
        "180",
        "--heartbeat-timeout",
        "120",
    ]
    if case.guest_vehicle is not None:
        command += ["--guest-vehicle", case.guest_vehicle]

    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_TRACE_ENHANCED_RENDERER": "1",
            "RECOMPONE_GPU_HLE": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_UNTHROTTLED": "1",
        }
    )
    for key in (
        "RECOMPONE_HEADLESS",
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE",
        "RECOMPONE_PRESENTATION_CAPTURE",
        "RECOMPONE_PRESENTATION_CAPTURE_FRAMES",
        "RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES",
    ):
        env.pop(key, None)

    stdout_path = case_dir / "matrix.stdout.log"
    stderr_path = case_dir / "matrix.stderr.log"
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    started = time.monotonic()
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        try:
            completed = subprocess.run(
                command,
                cwd=REPO,
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

    return collect_case_result(
        case, case_dir, exit_code, round(elapsed, 3)
    )


def collect_case_result(
    case: MatrixCase,
    case_dir: Path,
    exit_code: int,
    elapsed_seconds: float,
) -> CaseResult:
    summary_path = case_dir / "summary.json"
    summary = (
        json.loads(summary_path.read_text(encoding="utf-8"))
        if summary_path.is_file()
        else {}
    )
    runs = summary.get("runs", [])
    totals = summary.get("totals", {})
    soak_passed = (
        len(runs) == 1
        and bool(runs[0].get("passed"))
        and totals.get("runs") == 1
        and totals.get("passed") == 1
        and totals.get("failed") == 0
    )
    logs = sorted(case_dir.glob("*.stderr.log"))
    logs = [path for path in logs if path.name != "matrix.stderr.log"]
    renderer = analyze_renderer_log(logs[0]) if len(logs) == 1 else None
    renderer_passed = bool(renderer and renderer["passed"])
    passed = exit_code == 0 and soak_passed and renderer_passed
    return CaseResult(
        label=case.label,
        map_slot=case.map_slot,
        character_slot=case.character_slot,
        guest_vehicle=case.guest_vehicle,
        exit_code=exit_code,
        elapsed_seconds=elapsed_seconds,
        soak_passed=soak_passed,
        renderer_passed=renderer_passed,
        maximum_visible_world_fallback_percent=(
            renderer["maximum_visible_world_fallback_percent"]
            if renderer
            else None
        ),
        maximum_glass_primitives=(
            int(renderer["maximum_glass_primitives"])
            if renderer
            else 0
        ),
        stderr_log=str(logs[0]) if len(logs) == 1 else None,
        passed=passed,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--frames", type=int, default=600)
    parser.add_argument("--workers", type=int, default=3)
    parser.add_argument("--case-timeout", type=float, default=420.0)
    parser.add_argument(
        "--report-only",
        action="store_true",
        help="rebuild acceptance.json from already completed case directories",
    )
    parser.add_argument(
        "--resume",
        action="store_true",
        help="retain passing case directories and rerun only missing/failed cases",
    )
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    output = args.output.resolve()
    if not exe.is_file():
        raise FileNotFoundError(exe)
    if not (loose / "SLUS_008.68").is_file():
        raise FileNotFoundError(loose / "SLUS_008.68")
    output.mkdir(parents=True, exist_ok=True)

    cases = build_cases()
    results: list[CaseResult] = []
    if args.report_only:
        for case in cases:
            case_dir = output / case.label
            summary_path = case_dir / "summary.json"
            summary = (
                json.loads(summary_path.read_text(encoding="utf-8"))
                if summary_path.is_file()
                else {}
            )
            elapsed = float(
                (summary.get("runs") or [{}])[0].get("wall_seconds", 0.0)
            )
            results.append(
                collect_case_result(case, case_dir, 0, round(elapsed, 3))
            )
    else:
        pending_cases = cases
        if args.resume:
            pending_cases = []
            for case in cases:
                case_dir = output / case.label
                summary_path = case_dir / "summary.json"
                summary = (
                    json.loads(summary_path.read_text(encoding="utf-8"))
                    if summary_path.is_file()
                    else {}
                )
                elapsed = float(
                    (summary.get("runs") or [{}])[0].get(
                        "wall_seconds", 0.0)
                )
                existing = collect_case_result(
                    case, case_dir, 0, round(elapsed, 3))
                if existing.passed:
                    results.append(existing)
                else:
                    pending_cases.append(case)
        with concurrent.futures.ThreadPoolExecutor(
            max_workers=max(1, min(args.workers, len(pending_cases)))
        ) as executor:
            futures = {
                executor.submit(
                    run_case,
                    case,
                    exe,
                    loose,
                    output,
                    args.frames,
                    args.case_timeout,
                ): case
                for case in pending_cases
            }
            for future in concurrent.futures.as_completed(futures):
                results.append(future.result())

    for result in results:
        print(
            f"[EnhancedMatrix] case={result.label} pass={result.passed} "
            f"soak={result.soak_passed} renderer={result.renderer_passed} "
            f"visible_fallback=" +
            (
                f"{result.maximum_visible_world_fallback_percent:.3f}%"
                if result.maximum_visible_world_fallback_percent is not None
                else "missing"
            ),
            flush=True,
        )

    order = {case.label: index for index, case in enumerate(cases)}
    results.sort(key=lambda item: order[item.label])
    fallback_values = [
        value
        for result in results
        if (value := result.maximum_visible_world_fallback_percent) is not None
    ]
    report = {
        "schema": 1,
        "passed": all(result.passed for result in results),
        "cases_passed": sum(result.passed for result in results),
        "cases_total": len(results),
        "stock_cases": 18,
        "guest_cases": len(GUESTS),
        "frames_per_case": args.frames,
        "executable": str(exe),
        "executable_sha256":
            hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "maximum_visible_world_fallback_percent":
            max(fallback_values, default=None),
        "results": [asdict(result) for result in results],
    }
    report_path = output / "acceptance.json"
    report_path.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[EnhancedMatrix] pass={report['passed']} "
        f"cases={report['cases_passed']}/{report['cases_total']} "
        f"report={report_path}"
    )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
