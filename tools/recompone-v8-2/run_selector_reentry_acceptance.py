#!/usr/bin/env python3
"""Hidden-GL imported-selector accept/back/re-entry lifecycle acceptance."""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
BLENDER_PYTHON = Path(
    r"C:\Program Files\Blender Foundation\Blender 4.5\4.5\python\bin\python.exe"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--exe",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "Vigilante82PC.exe",
    )
    parser.add_argument(
        "--loose", type=Path, default=REPO / "V8_2_LOOSE"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts" / "v82-selector-reentry",
    )
    parser.add_argument("--timeout", type=float, default=180.0)
    args = parser.parse_args()
    exe, loose, output = (
        args.exe.resolve(), args.loose.resolve(), args.output.resolve()
    )
    output.mkdir(parents=True, exist_ok=True)
    for stale in (
        *output.glob("*.ppm"),
        output / "stdout.log",
        output / "stderr.log",
        output / "acceptance.json",
        output / "selector-generation-contact.bmp",
    ):
        if stale.is_file():
            stale.unlink()
    fixture = (
        Path(__file__).with_name("input-scripts")
        / "native_guest_enemy_back_reenter_probe.txt"
    ).resolve()
    stdout_path, stderr_path = output / "stdout.log", output / "stderr.log"
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_INPUT_FILE": str(fixture),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_GPU_HLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_UNTHROTTLED": "0",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "4600",
            "RECOMPONE_PRESENTATION_CAPTURE": "1",
            "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "1",
            "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
            "RECOMPONE_CAPTURE_SELECTOR_GENERATIONS": "1",
            "RECOMPONE_TRACE_V82_SELECTOR": "1",
            "RECOMPONE_CAPTURE_DIR": str(output),
        }
    )
    env.pop("RECOMPONE_HEADLESS", None)
    with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
        try:
            completed = subprocess.run(
                [str(exe), "--loose", str(loose)],
                cwd=exe.parent,
                env=env,
                stdout=stdout,
                stderr=stderr,
                timeout=args.timeout,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                check=False,
            )
            exit_code = completed.returncode
        except subprocess.TimeoutExpired:
            exit_code = 124
    text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in (stdout_path, stderr_path)
    )
    generations = sorted(
        set(
            re.findall(
                r"captured presentation "
                r"'native_guest_11_generation_(\d{3})'",
                text,
            )
        )
    )
    built = len(
        re.findall(
            r"\[V82Vehicles\] built guest\.v8\.sid_burn selector=", text
        )
    )
    released = len(
        re.findall(
            r"\[V82SelectorResources\] released=guest\.v8\.sid_burn ", text
        )
    )
    generation_images = []
    for generation in generations:
        stem = (
            "recompone_present_native_guest_11_generation_"
            f"{generation}_1280x720_"
        )
        matches = sorted(output.glob(stem + "*.ppm"))
        generation_images.append(
            matches[0] if matches else output / (stem + "fxaa.ppm")
        )
    contact_sheet = output / "selector-generation-contact.bmp"
    if len(generation_images) >= 2 and all(
        image.is_file() for image in generation_images
    ):
        subprocess.run(
            [
                str(BLENDER_PYTHON),
                str(Path(__file__).with_name("ppm_contact_sheet.py")),
                "--columns",
                "2",
                "--cell-width",
                "720",
                str(contact_sheet),
                *map(str, generation_images[:2]),
            ],
            check=True,
        )
    checks = {
        "clean_exit": exit_code == 0,
        "enemy_selector_reached": "[Input] stage 'choose_enemies'" in text,
        "player_selector_reentered":
            text.count("[Input] stage 'choose_player'") >= 2,
        "sid_rebuilt": built >= 2,
        "sid_released_each_time": released == built,
        "two_visual_generations": len(generations) >= 2,
        "generation_contact_sheet": contact_sheet.is_file(),
        "no_fatal":
            "[Fatal]" not in text and "Unhandled exception" not in text,
    }
    report = {
        "passed": all(checks.values()),
        "checks": checks,
        "exit_code": exit_code,
        "sid_builds": built,
        "sid_releases": released,
        "visual_generations": generations,
        "visual_contact_sheet": str(contact_sheet),
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    state = "PASS" if report["passed"] else "FAIL"
    print(
        f"[SelectorReentry] {state} builds={built} releases={released} "
        f"generations={','.join(generations)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    for capture in output.glob("*.ppm"):
        capture.unlink()
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
