#!/usr/bin/env python3
"""Verify one long physical selector hold leaves Y visibly stable."""

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
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_INPUT = (
    Path(__file__).with_name("input-scripts")
    / "native_y_held_selector_gate.txt"
)
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-y-selector-held-input"
EXIT_AFTER_POLLS = 5200
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
            chunks.append(path.read_text(
                encoding="utf-8", errors="replace"))
        except FileNotFoundError:
            pass
    return "\n".join(chunks)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--timeout", type=float, default=360.0)
    parser.add_argument("--heartbeat-timeout", type=float, default=45.0)
    parser.add_argument(
        "--visual",
        action="store_true",
        help="capture the early, settled, and proof frames for Y",
    )
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    fixture = args.input.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for name in ("stdout.log", "stderr.log", "runtime.log", "acceptance.json"):
        path = output / name
        if path.is_file():
            path.unlink()
    for path in output.glob("*.ppm"):
        path.unlink()
    if not exe.is_file():
        raise FileNotFoundError(exe)
    if not fixture.is_file():
        raise FileNotFoundError(fixture)

    stdout_path = output / "stdout.log"
    stderr_path = output / "stderr.log"
    runtime_path = output / "runtime.log"
    env = os.environ.copy()
    env.update({
        "RECOMPONE_INPUT_FILE": str(fixture),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_GPU_HLE": "1",
        "RECOMPONE_MUTE": "1",
        "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_UNTHROTTLED": "0",
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": str(EXIT_AFTER_POLLS),
        "RECOMPONE_PRESENTATION_CAPTURE": "1" if args.visual else "0",
        "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
        "RECOMPONE_DISPLAY_PROBE_IMAGES": "0",
        "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": (
            "1" if args.visual else "0"
        ),
        "RECOMPONE_CAPTURE_V82_SELECTOR_SETTLE": (
            "1" if args.visual else "0"
        ),
        "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
        "RECOMPONE_CAPTURE_DIR": str(output),
        "RECOMPONE_TRACE_V82_SELECTOR": "1",
        "RECOMPONE_TRACE_V82_SELECTOR_PHYSICS": "1",
        "RECOMPONE_LOG_PATH": str(runtime_path),
    })
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
                    for path in (stdout_path, stderr_path, runtime_path)
                    if path.exists()
                )
                if size != last_size:
                    last_size = size
                    last_progress = time.monotonic()
                text = combined_text(
                    stdout_path, stderr_path, runtime_path).lower()
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
                        process, output, "selector-hold")
                    break
                if time.monotonic() - last_progress > args.heartbeat_timeout:
                    reason = "selector hold heartbeat timeout"
                    hang_stack = capture_managed_stack(
                        process, output, "selector-hold")
                    break
                time.sleep(0.5)
        finally:
            if reason:
                stop_process(process)
            else:
                process.wait(timeout=10)

    # Runtime diagnostics are mirrored to stderr in desktop builds. Parse one
    # authoritative stream so a valid single carousel step is not counted
    # twice merely because logging was intentionally verbose.
    text = runtime_path.read_text(
        encoding="utf-8", errors="replace"
    ) if runtime_path.is_file() else combined_text(stderr_path, stdout_path)
    carousel_guests = [
        int(value)
        for value in re.findall(
            r"\[V82SelectorCarousel\] step direction=(?:left|right) "
            r"from=(?:retail|guest)\.\d+ to=guest\.(\d+)",
            text,
        )
    ]
    releases = [
        (direction, int(suppressed), int(guest))
        for direction, suppressed, guest in re.findall(
            r"\[V82SelectorCarousel\] release direction=(left|right) "
            r"suppressed=(\d+) guest=(-?\d+)(?: raw-pad=0x[0-9A-Fa-f]+)?",
            text,
        )
    ]
    held_y_release = any(
        direction == "left" and guest == 12
        for direction, suppressed, guest in releases
    )
    preview_builds = re.findall(
        r"\[V82Vehicles\] built (guest\.v8\.[a-z0-9_]+) selector=",
        text,
    )
    physics_rows = [
        {
            "frame": int(frame),
            "y": int(y),
            "vy": int(vy),
            "supports": len(re.findall(r" w[0-5]=0x", tail)),
        }
        for frame, y, vy, tail in re.findall(
            r"\[V82SelectorPhysics\] guest=12 frame=(\d+) "
            r"vehicle=0x[0-9A-Fa-f]+ pos=\(-?\d+,(-?\d+),-?\d+\) "
            r"vel=\(-?\d+,(-?\d+),-?\d+\)([^\r\n]*)",
            text,
        )
    ]
    initial_physics = physics_rows[0] if physics_rows else None
    final_physics = physics_rows[-1] if physics_rows else None
    physical_hold = (
        initial_physics is not None
        and final_physics is not None
        and final_physics["frame"] >= 200
        and all(row["supports"] == 4 for row in physics_rows)
        and abs(final_physics["y"] - initial_physics["y"]) <= 0x40000
        and abs(final_physics["vy"]) <= 0x10000
    )
    clean_exit = (
        process.returncode == 0
        and f"deterministic replay completed at poll {EXIT_AFTER_POLLS}"
        in text
    )
    passed = (
        not reason
        and carousel_guests == [12]
        and held_y_release
        and preview_builds == ["guest.v8.y_the_alien"]
        and physical_hold
        and clean_exit
    )
    if not reason and carousel_guests != [12]:
        reason = f"long LEFT hold visited guest sequence {carousel_guests}"
    if not reason and not held_y_release:
        reason = f"long LEFT hold did not release while Y was active: {releases}"
    if not reason and preview_builds != ["guest.v8.y_the_alien"]:
        reason = f"long LEFT hold built previews {preview_builds}"
    if not reason and not physical_hold:
        reason = (
            "Y did not remain physically supported: "
            f"initial={initial_physics} final={final_physics} "
            f"rows={len(physics_rows)}"
        )
    if not reason and not clean_exit:
        reason = f"process exit was not clean: {process.returncode}"

    captured_frames = sorted(path.name for path in output.glob("*.ppm"))
    report = {
        "schema": 1,
        "passed": passed,
        "reason": reason or "one long LEFT hold remained on Y",
        "executable": str(exe),
        "executable_sha256": hashlib.sha256(
            exe.read_bytes()).hexdigest().upper(),
        "fixture": str(fixture),
        "carousel_guests": carousel_guests,
        "releases": releases,
        "held_y_release": held_y_release,
        "preview_builds": preview_builds,
        "physics_rows": len(physics_rows),
        "initial_physics": initial_physics,
        "final_physics": final_physics,
        "physical_hold": physical_hold,
        "clean_exit": clean_exit,
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "hang_stack": str(hang_stack) if hang_stack else None,
        "captured_frames": captured_frames,
        "retained_images": len(captured_frames),
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
