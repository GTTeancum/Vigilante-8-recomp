#!/usr/bin/env python3
"""Hidden native-selector lifecycle acceptance for the staged V8:2 build."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
import re
import subprocess
import sys
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
EXIT_AFTER_POLLS = 12800


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
    parser.add_argument(
        "--complete-carousel",
        action="store_true",
        help=(
            "verify a complete selectable-character lap in both directions: "
            "the native mode mask plus all 13 imported entries"
        ),
    )
    parser.add_argument(
        "--exit-after-polls", type=int, default=EXIT_AFTER_POLLS,
    )
    parser.add_argument(
        "--text-only",
        action="store_true",
        help=(
            "verify the complete selector lifecycle from verbose runtime "
            "diagnostics without producing presentation captures"
        ),
    )
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
        output / "runtime.log",
        output / "acceptance.json",
        output / "audit.json",
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
            "RECOMPONE_UNTHROTTLED": "0",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": str(EXIT_AFTER_POLLS),
            "RECOMPONE_PRESENTATION_CAPTURE": (
                "0" if args.text_only else "1"
            ),
            "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": (
                "0" if args.text_only else "1"
            ),
            "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
            "RECOMPONE_CAPTURE_DIR": str(output),
            "RECOMPONE_TRACE_VEHICLE_MATERIALS": "1",
            "RECOMPONE_TRACE_SELECTOR_RENDER_STATE": "1",
            "RECOMPONE_TRACE_PACKET_ARENAS": "1",
            "RECOMPONE_TRACE_V82_SELECTOR": "1",
            "RECOMPONE_TRACE_V82_SELECTOR_PHYSICS": "1",
            "RECOMPONE_TRACE_NATIVE_OPTIONS": "1",
            "RECOMPONE_DISPLAY_PROBE_IMAGES": (
                "0" if args.text_only else "1"
            ),
            "RECOMPONE_LOG_PATH": str(output / "runtime.log"),
        }
    )
    env.pop("RECOMPONE_HEADLESS", None)
    env["RECOMPONE_SCRIPT_EXIT_AFTER_POLLS"] = str(args.exit_after_polls)
    if args.complete_carousel:
        env["RECOMPONE_V82_UNLOCK_ROSTER"] = "1"

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
    preview_build_counts = Counter(re.findall(
        r"\[V82Vehicles\] built (guest\.v8\.[a-z0-9_]+) selector=",
        text,
    ))
    carousel_guests = [
        int(value)
        for value in re.findall(
            r"\[V82SelectorCarousel\] step direction=(?:left|right) "
            r"from=(?:retail|guest)\.\d+ to=guest\.(\d+)",
            text,
        )
    ]
    carousel_transitions = [
        (direction, source, target)
        for direction, source, target in re.findall(
            r"\[V82SelectorCarousel\] step direction=(left|right) "
            r"from=((?:retail|guest)\.\d+) "
            r"to=((?:retail|guest)\.\d+)",
            text,
        )
    ]
    expected_carousel_guests = [
        *range(12, -1, -1),
        *range(1, 13),
        *range(11, -1, -1),
    ]
    carousel_releases = [
        (direction, int(suppressed), int(guest))
        for direction, suppressed, guest in re.findall(
            r"\[V82SelectorCarousel\] release direction=(left|right) "
            r"suppressed=(\d+) guest=(-?\d+)",
            text,
        )
    ]
    held_step_suppressed = any(
        suppressed > 0
        for _, suppressed, guest in carousel_releases
        if guest >= 0
    )
    # SHELL's Arcade player mask at this call site is 0x00038E38. The native
    # selector treats its set bits as unavailable, so the selectable retail
    # route is nine entries from the eighteen-type engine table. Preserve the
    # native skip decisions; the imported roster extends that accepted route.
    selectable_retail = [0, 1, 2, 6, 7, 8, 12, 13, 14]
    expected_complete_transitions = [
        *[("right", f"retail.{source}", f"retail.{target}")
          for source, target in zip(
              selectable_retail, selectable_retail[1:])],
        ("right", "retail.14", "guest.0"),
        *[("right", f"guest.{index}", f"guest.{index + 1}")
          for index in range(12)],
        ("right", "guest.12", "retail.0"),
        ("left", "retail.0", "guest.12"),
        *[("left", f"guest.{index}", f"guest.{index - 1}")
          for index in range(12, 0, -1)],
        ("left", "guest.0", "retail.14"),
        *[("left", f"retail.{source}", f"retail.{target}")
          for source, target in zip(
              reversed(selectable_retail),
              list(reversed(selectable_retail))[1:])],
    ]
    carousel_order_passed = (
        carousel_transitions == expected_complete_transitions
        if args.complete_carousel
        else carousel_guests == expected_carousel_guests
    )
    complete_double_roster = (
        len(preview_build_counts) == 13
        and all(count >= 2 for count in preview_build_counts.values())
    )
    clean_exit = (
        process.returncode == 0
        and (
            f"deterministic replay completed at poll {args.exit_after_polls}"
            in text
        )
    )
    passed = (
        not reason
        and (args.text_only or captures == list(range(13)))
        and complete_double_roster
        and carousel_order_passed
        and enemy_stage
        and (args.text_only or enemy_capture)
        and clean_exit
    )
    if not args.text_only and not reason and captures != list(range(13)):
        reason = f"selector captures {captures}, expected 0-12"
    if not reason and not complete_double_roster:
        reason = (
            "selector did not construct every imported preview twice: "
            f"{dict(sorted(preview_build_counts.items()))}"
        )
    if not reason and not carousel_order_passed:
        reason = (
            "selector carousel order did not match the expected route: "
            f"actual={carousel_transitions if args.complete_carousel else carousel_guests} "
            f"expected={expected_complete_transitions if args.complete_carousel else expected_carousel_guests}"
        )
    if not reason and not enemy_stage:
        reason = "enemy selector was not reached"
    if not args.text_only and not reason and not enemy_capture:
        reason = "enemy selector proof frame was not captured"
    if not reason and not clean_exit:
        reason = f"process exit was not clean: {process.returncode}"

    guest_images = []
    for index in range(13):
        matches = sorted(output.glob(
            f"recompone_present_native_guest_{index:02}_1280x720_*.ppm"
        ))
        if len(matches) == 1:
            guest_images.append(matches[0])
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
    if not args.text_only and len(guest_images) == 13 and all(
            image.is_file() for image in guest_images):
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
    if not args.text_only and not reason and not contact_sheet.is_file():
        reason = "full-roster visual contact sheet was not produced"
        passed = False

    churn_report = output / "audit.json"
    if args.text_only:
        churn_exit = subprocess.run(
            [
                sys.executable,
                str(Path(__file__).with_name(
                    "analyze_selector_render_churn.py")),
                str(output / "runtime.log"),
                "--output",
                str(churn_report),
                "--minimum-generations",
                "2",
            ],
            check=False,
        ).returncode
        if churn_exit != 0:
            reason = "selector render/lifecycle churn audit failed"
            passed = False

    report = {
        "schema": 1,
        "passed": passed,
        "text_only": args.text_only,
        "reason": reason or "complete native selector lifecycle",
        "executable": str(exe),
        "executable_sha256":
            hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "captures": captures,
        "preview_build_counts": dict(sorted(preview_build_counts.items())),
        "complete_double_roster": complete_double_roster,
        "carousel_guests": carousel_guests,
        "expected_carousel_guests": expected_carousel_guests,
        "complete_carousel": args.complete_carousel,
        "selectable_retail_slots": (
            selectable_retail if args.complete_carousel else []
        ),
        "carousel_transitions": carousel_transitions,
        "expected_complete_transitions": (
            expected_complete_transitions if args.complete_carousel else []
        ),
        "carousel_order_passed": carousel_order_passed,
        "carousel_releases": carousel_releases,
        "held_step_suppressed": held_step_suppressed,
        "enemy_stage": enemy_stage,
        "enemy_capture": enemy_capture,
        "clean_exit": clean_exit,
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "hang_stack": str(hang_stack) if hang_stack else None,
        "visual_contact_sheet": str(contact_sheet),
        "vehicle_material_report": str(material_report),
        "vehicle_materials_passed": material_exit == 0,
        "selector_churn_report": (
            str(churn_report) if args.text_only else None
        ),
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    for capture in output.glob("*.ppm"):
        capture.unlink()
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
