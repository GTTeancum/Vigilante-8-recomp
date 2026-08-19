#!/usr/bin/env python3
"""Accept 16:9 gameplay returning to a clean, centered 4:3 menu.

This targets the actual regression: after widescreen gameplay contracts to
the authored 4:3 shell, the newly exposed left/right host regions must be
explicitly black.  Retained gameplay pixels in those regions are HOM.
"""

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
        default=(
            REPO / "reference-v8-2" / "generated" / "recompiled"
            / "bin" / "Release" / "net10.0" / "win-x64"
            / "Vigilante82PC.exe"
        ),
    )
    parser.add_argument(
        "--loose", type=Path, default=REPO / "V8_2_LOOSE"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts" / "v82-widescreen-menu-return",
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
        output / "shell-transition-pixels.json",
        output / "shell-menu-return-pixels.json",
        output / "widescreen-to-four-three-menu-pixels.json",
    ):
        if stale.is_file():
            stale.unlink()
    fixture = (
        Path(__file__).with_name("input-scripts")
        / "native_quest_defeat_return.txt"
    ).resolve()
    stdout_path, stderr_path = output / "stdout.log", output / "stderr.log"
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_INPUT_FILE": str(fixture),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_OUTPUT_RESOLUTION": "1280x720",
            "RECOMPONE_GPU_HLE": "1",
            "RECOMPONE_V82_TEST_DEFEAT_FRAME": "120",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "6000",
            "RECOMPONE_V82_UNLOCK_ROSTER": "1",
            "RECOMPONE_TRACE_RESULTS": "1",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_UNTHROTTLED": "0",
            "RECOMPONE_PRESENTATION_CAPTURE": "1",
            # The native result lock, prompt, and X-to-return input consume
            # roughly 825 presented frames. Retain enough uninterrupted
            # composition metrics for 240 contracted 4:3 frames plus margin.
            "RECOMPONE_PRESENTATION_CAPTURE_BURST_FRAMES": "1200",
            "RECOMPONE_PRESENTATION_CAPTURE_BURST_LABEL":
                "defeated",
            "RECOMPONE_CAPTURE_DIR": str(output),
            "RECOMPONE_COMPOSED_PRESENTATION_CAPTURE": "1",
            "RECOMPONE_COMPOSED_PRESENTATION_CAPTURE_STRIDE": "30",
            # The ordinary stage capture supplies the wide source sample.
            # Saving every full-resolution source frame would create several
            # gigabytes of redundant screenshots; only the composed stride
            # samples and per-frame edge metrics are needed for the boundary.
            "RECOMPONE_SOURCE_PRESENTATION_CAPTURE": "0",
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
    combined = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in (stdout_path, stderr_path)
    )
    transition_captures = sorted(
        output.glob(
            "recompone_present_defeated_*_fxaa.ppm"
        )
    )
    captures = sorted(
        output.glob(
            "recompone_composed_defeated_*.ppm"
        )
    )
    pixel_report = output / "widescreen-to-four-three-menu-pixels.json"
    pixel_exit = subprocess.run(
        [
            str(BLENDER_PYTHON),
            str(Path(__file__).with_name("analyze_capture_edges.py")),
            "--mode",
            "shell-menu",
            "--output",
            str(pixel_report),
            "--layout-log",
            str(stdout_path),
            *map(str, captures),
        ],
        check=False,
    ).returncode if captures else 2
    pixel_data = (
        json.loads(pixel_report.read_text(encoding="utf-8"))
        if pixel_report.is_file() else {}
    )
    contact_sheet = output / "widescreen-to-four-three-menu-contact.bmp"
    # This burst begins while the defeated/result presentation is still the
    # wide gameplay surface.  Its first source frame is therefore the exact
    # pre-contraction proof; looking for an unrelated "gameplay" label could
    # silently select no frame or a stale capture.
    wide_source_samples = transition_captures
    menu_samples = [
        path for path in captures
        if any(
            frame.get("file") == str(path)
            and bool(frame.get("menu_content_visible", False))
            for frame in pixel_data.get("captures", [])
        )
    ]
    if wide_source_samples and len(menu_samples) >= 2:
        subprocess.run(
            [
                str(BLENDER_PYTHON),
                str(Path(__file__).with_name("ppm_contact_sheet.py")),
                "--columns",
                "3",
                "--cell-width",
                "480",
                str(contact_sheet),
                str(wide_source_samples[0]),
                str(menu_samples[0]),
                str(menu_samples[-1]),
            ],
            check=True,
        )
    aspect_events = [
        {
            "offset": match.start(),
            "aspect": float(match.group("aspect")),
            "gameplay": int(match.group("gameplay")),
        }
        for match in re.finditer(
            r"\[OutputAspect\] texture=\d+x\d+ "
            r"aspect=(?P<aspect>\d+(?:\.\d+)?) "
            r"gameplay=(?P<gameplay>[01])",
            combined,
        )
    ]
    wide_events = [
        event for event in aspect_events
        if event["gameplay"] == 1 and event["aspect"] >= 1.70
    ]
    returned_menu_events = [
        event for event in aspect_events
        if event["gameplay"] == 0
        and 1.30 <= event["aspect"] <= 1.36
        and wide_events
        and event["offset"] > wide_events[0]["offset"]
    ]
    pixel_checks = pixel_data.get("checks", {})
    checks = {
        "clean_exit": exit_code == 0,
        "native_lethal_damage":
            "[V82DefeatRegression] native lethal damage completed"
            in combined,
        "result_screen_built":
            "[V82ResultString] sprintf caller=0x80013268" in combined,
        "shell_overlay_loaded":
            "loaded relocated overlay: SHELL_SHELL" in combined,
        "shell_transition_signaled":
            combined.rfind("[Input] stage 'shell_transition'")
            > combined.rfind("[Input] stage 'defeated'"),
        "stale_widescreen_target_discarded_without_erasing_menu_vram":
            "[WidescreenMenuReturn] invalidated stale 16:9 output"
            in combined,
        "menu_music_recovered":
            "[CDDA] loose track=2 source=music/track02.ogg streaming"
            in combined,
        "widescreen_gameplay_observed": bool(wide_events),
        "four_three_menu_observed_after_widescreen":
            bool(returned_menu_events),
        "newly_exposed_left_and_right_bands_are_black":
            pixel_exit == 0
            and bool(pixel_checks.get(
                "all_returned_menu_bars_black", False
            )),
        "four_three_menu_is_visible_inside_black_bands":
            bool(pixel_checks.get("stable_menu_frames_observed", False))
            and bool(pixel_checks.get("final_frame_is_menu", False)),
        "full_host_composition_captured":
            len(pixel_data.get("frames", [])) >= 300
            and len(captures) >= 3,
        "visual_contact_sheet":
            contact_sheet.is_file()
            and bool(wide_source_samples)
            and len(menu_samples) >= 2,
        "replay_completed":
            "[Input] deterministic replay completed at poll 6000"
            in combined,
        "no_fatal":
            "[Fatal]" not in combined
            and "Unhandled exception" not in combined,
    }
    report = {
        "schema": 2,
        "acceptance": "16:9 gameplay to centered 4:3 menu without HOM",
        "passed": all(checks.values()),
        "checks": checks,
        "exit_code": exit_code,
        "capture_count": len(captures),
        "source_transition_capture_count":
            len(transition_captures),
        "source_wide_result_capture_count": len(wide_source_samples),
        "composed_menu_sample_count": len(menu_samples),
        "presentation_aspect_events": aspect_events,
        "pixel_report": str(pixel_report),
        "visual_contact_sheet": str(contact_sheet),
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[WidescreenToFourThreeMenu] "
        f"{'PASS' if report['passed'] else 'FAIL'} "
        f"captures={len(captures)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    for capture in output.glob("*.ppm"):
        capture.unlink()
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
