#!/usr/bin/env python3
"""Hidden native-selector lifecycle acceptance for the staged V8:2 build."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import time
from pathlib import Path

from run_reference_soak import capture_managed_stack, stop_process


REPO = Path(__file__).resolve().parents[2]
BLENDER_PYTHON = Path(
    r"C:\Program Files\Blender Foundation\Blender 4.5\4.5\python\bin\python.exe"
)
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_INPUT = (
    Path(__file__).with_name("input-scripts")
    / "native_full_roster_selector_proof.txt"
)
DEFAULT_OUTPUT = (
    REPO / "artifacts" / "renderer-rewrite-selector-acceptance"
)
FATAL_MARKERS = (
    "[fatal]",
    "unhandled exception",
    "unmapped call",
    "out of vram",
    "outofmemoryexception",
)


def combined_text(*paths: Path) -> str:
    chunks: list[str] = []
    for path in paths:
        try:
            chunks.append(path.read_text(encoding="utf-8", errors="replace"))
        except FileNotFoundError:
            pass
    return "\n".join(chunks)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--timeout", type=float, default=420.0)
    parser.add_argument("--heartbeat-timeout", type=float, default=45.0)
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    fixture = args.input.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for stale in (
        *output.glob("*.ppm"),
        output / "stdout.log",
        output / "stderr.log",
        output / "acceptance.json",
        output / "all-guests-contact.bmp",
        output / "vehicle-materials.json",
    ):
        if stale.is_file():
            stale.unlink()
    if not exe.is_file():
        raise FileNotFoundError(exe)
    if not (loose / "SLUS_008.68").is_file():
        raise FileNotFoundError(loose / "SLUS_008.68")
    if not fixture.is_file():
        raise FileNotFoundError(fixture)

    stdout_path = output / "stdout.log"
    stderr_path = output / "stderr.log"
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_INPUT_FILE": str(fixture),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_GPU_HLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_UNTHROTTLED": "1",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "5300",
            "RECOMPONE_PRESENTATION_CAPTURE": "1",
            "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "1",
            "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
            "RECOMPONE_CAPTURE_DIR": str(output),
            "RECOMPONE_TRACE_VEHICLE_MATERIALS": "1",
        }
    )
    env.pop("RECOMPONE_HEADLESS", None)

    started = time.monotonic()
    last_progress = started
    last_size = -1
    reason = ""
    hang_stack: Path | None = None
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        process = subprocess.Popen(
            [str(exe), "--loose", str(loose)],
            cwd=exe.parent,
            env=env,
            stdout=stdout,
            stderr=stderr,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        try:
            while process.poll() is None:
                elapsed = time.monotonic() - started
                size = sum(
                    path.stat().st_size
                    for path in (stdout_path, stderr_path)
                    if path.exists()
                )
                if size != last_size:
                    last_size = size
                    last_progress = time.monotonic()
                text = combined_text(stdout_path, stderr_path).lower()
                marker = next(
                    (value for value in FATAL_MARKERS if value in text),
                    None,
                )
                if marker is not None:
                    reason = f"fatal marker: {marker}"
                    break
                if elapsed > args.timeout:
                    reason = "overall timeout"
                    hang_stack = capture_managed_stack(
                        process, output, "selector")
                    break
                if (
                    time.monotonic() - last_progress
                    > args.heartbeat_timeout
                ):
                    reason = "selector heartbeat timeout"
                    hang_stack = capture_managed_stack(
                        process, output, "selector")
                    break
                time.sleep(0.5)
        finally:
            if reason:
                stop_process(process)
            else:
                process.wait(timeout=10)

    text = combined_text(stdout_path, stderr_path)
    captures = sorted(
        {
            int(value)
            for value in re.findall(
                r"captured presentation 'native_guest_(\d{2})'",
                text,
            )
        }
    )
    enemy_stage = "[Input] stage 'choose_enemies'" in text
    enemy_capture = (
        "captured presentation 'native_enemy_selector'" in text
    )
    clean_exit = (
        process.returncode == 0
        and "deterministic replay completed at poll 5300" in text
    )
    passed = (
        not reason
        and captures == list(range(12))
        and enemy_stage
        and enemy_capture
        and clean_exit
    )
    if not reason and captures != list(range(12)):
        reason = f"selector captures {captures}, expected 0-11"
    if not reason and not enemy_stage:
        reason = "enemy selector was not reached"
    if not reason and not enemy_capture:
        reason = "enemy selector proof frame was not captured"
    if not reason and not clean_exit:
        reason = f"process exit was not clean: {process.returncode}"

    guest_images = [
        output
        / (
            f"recompone_present_native_guest_{index:02}_"
            "1280x720_fxaa.ppm"
        )
        for index in range(12)
    ]
    contact_sheet = output / "all-guests-contact.bmp"
    material_report = output / "vehicle-materials.json"
    material_exit = subprocess.run(
        [
            str(BLENDER_PYTHON),
            str(Path(__file__).with_name(
                "analyze_vehicle_material_trace.py")),
            str(stderr_path),
            "--output",
            str(material_report),
        ],
        check=False,
    ).returncode
    if material_exit != 0:
        reason = (
            "vehicle material audit found a subtractive effect "
            "classified as glass"
        )
        passed = False
    if all(image.is_file() for image in guest_images):
        subprocess.run(
            [
                str(BLENDER_PYTHON),
                str(Path(__file__).with_name("ppm_contact_sheet.py")),
                "--columns",
                "4",
                "--cell-width",
                "480",
                str(contact_sheet),
                *map(str, guest_images),
            ],
            check=True,
        )
    if not reason and not contact_sheet.is_file():
        reason = "full-roster visual contact sheet was not produced"
        passed = False

    report = {
        "schema": 1,
        "passed": passed,
        "reason": reason or "complete native selector lifecycle",
        "executable": str(exe),
        "executable_sha256":
            hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "captures": captures,
        "enemy_stage": enemy_stage,
        "enemy_capture": enemy_capture,
        "clean_exit": clean_exit,
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "hang_stack": str(hang_stack) if hang_stack else None,
        "visual_contact_sheet": str(contact_sheet),
        "vehicle_material_report": str(material_report),
        "vehicle_materials_passed": material_exit == 0,
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    for capture in output.glob("*.ppm"):
        capture.unlink()
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
