#!/usr/bin/env python3
"""Hidden native enemy-selector and guest-NPC gameplay acceptance."""

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
    / "native_guest_npc_selector_probe.txt"
)
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-open020-npc-acceptance"
FATAL_MARKERS = (
    "[fatal]",
    "unhandled exception",
    "unmapped call",
    "out of vram",
    "outofmemoryexception",
)


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    tokens: list[bytes] = []
    offset = 0
    while len(tokens) < 4:
        while offset < len(data) and data[offset] in b" \t\r\n":
            offset += 1
        if offset < len(data) and data[offset] == ord("#"):
            offset = data.index(b"\n", offset) + 1
            continue
        end = offset
        while end < len(data) and data[end] not in b" \t\r\n":
            end += 1
        tokens.append(data[offset:end])
        offset = end
    if tokens[0] != b"P6" or tokens[3] != b"255":
        raise ValueError(f"unsupported PPM header in {path}")
    while offset < len(data) and data[offset] in b" \t\r\n":
        offset += 1
    width, height = int(tokens[1]), int(tokens[2])
    pixels = data[offset:]
    if len(pixels) != width * height * 3:
        raise ValueError(f"truncated PPM pixels in {path}")
    return width, height, pixels


def visual_metrics(output: Path) -> tuple[int, int]:
    preview_hashes: set[str] = set()
    quantity_band_white: list[int] = []
    for index in range(12):
        path = output / (
            f"recompone_present_native_npc_{index:02d}_1280x720_off.ppm"
        )
        width, height, pixels = read_ppm(path)
        preview = bytearray()
        for y in range(20, min(130, height)):
            start = (y * width + 60) * 3
            end = (y * width + min(460, width)) * 3
            preview.extend(pixels[start:end])
        preview_hashes.add(hashlib.sha256(preview).hexdigest())

        white = 0
        for y in range(130, min(190, height)):
            for x in range(0, min(480, width)):
                pixel = (y * width + x) * 3
                if all(value > 190 for value in pixels[pixel:pixel + 3]):
                    white += 1
        quantity_band_white.append(white)
    return len(preview_hashes), max(quantity_band_white)


def read_text(*paths: Path) -> str:
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
    parser.add_argument("--heartbeat-timeout", type=float, default=60.0)
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    fixture = args.input.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for stale in (*output.glob("*.ppm"), *output.glob("*.bmp"),
                  output / "stdout.log", output / "stderr.log",
                  output / "acceptance.json"):
        if stale.is_file():
            stale.unlink()

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
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "15000",
            "RECOMPONE_PRESENTATION_CAPTURE": "1",
            "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
            "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "1",
            "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
            "RECOMPONE_TRACE_HUD": "1",
            "RECOMPONE_CAPTURE_DIR": str(output),
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
                text = read_text(stdout_path, stderr_path).lower()
                marker = next(
                    (value for value in FATAL_MARKERS if value in text), None)
                if marker:
                    reason = f"fatal marker: {marker}"
                    break
                if elapsed > args.timeout:
                    reason = "overall timeout"
                    hang_stack = capture_managed_stack(process, output, "npc")
                    break
                if time.monotonic() - last_progress > args.heartbeat_timeout:
                    reason = "NPC selector heartbeat timeout"
                    hang_stack = capture_managed_stack(process, output, "npc")
                    break
                time.sleep(0.5)
        finally:
            if reason:
                stop_process(process)
            else:
                process.wait(timeout=10)

    text = read_text(stdout_path, stderr_path)
    captures = sorted({
        int(value) for value in re.findall(
            r"captured presentation 'native_npc_(\d{2})'", text)
    })
    traversed = sorted({
        int(value) for value in re.findall(
            r"\[V82NpcSelector\] row=\d+ guest=(\d+)", text)
    })
    injected_types = sorted({
        int(value) for value in re.findall(
            r"\[V82Vehicles\] NPC participant=\d+ row=\d+ "
            r"proxy=\d+ type=(\d+)", text)
    })
    npc_objects = re.findall(
        r"\[V82Vehicles\] created (guest\.[a-z0-9_.-]+) "
        r"identity=(\d+) object=0x[0-9A-F]+", text)
    status_geometry = sorted(set(re.findall(
        r"\[V82HudPacket\].*status-backing=1.*"
        r"draw-x=([0-9.-]+) draw-w=(\d+)", text)))
    target_icon_geometry = sorted(set(re.findall(
        r"\[V82HudPacket\].*xy=(\d+),(?:22|262) wh=(\d+)x16.*"
        r"status-backing=0", text)))
    gameplay = "[Input] stage 'gameplay'" in text
    clean_exit = (
        process.returncode == 0
        and "deterministic replay completed at poll 15000" in text
    )
    expected_guests = list(range(12))
    try:
        unique_preview_frames, max_quantity_band_white = visual_metrics(output)
    except (FileNotFoundError, ValueError) as error:
        unique_preview_frames = 0
        max_quantity_band_white = 1_000_000
        if not reason:
            reason = f"visual evidence invalid: {error}"
    passed = (
        not reason
        and captures == expected_guests
        and traversed == expected_guests
        and unique_preview_frames >= 10
        and max_quantity_band_white < 1600
        and bool(injected_types)
        and len(npc_objects) == 1
        and ("74", "90") in status_geometry
        and ("76", "40") in target_icon_geometry
        and gameplay
        and clean_exit
    )
    if not reason and captures != expected_guests:
        reason = f"NPC captures {captures}, expected {expected_guests}"
    if not reason and traversed != expected_guests:
        reason = f"NPC traversal {traversed}, expected {expected_guests}"
    if not reason and unique_preview_frames < 10:
        reason = f"only {unique_preview_frames} distinct NPC preview frames"
    if not reason and max_quantity_band_white >= 1600:
        reason = (
            "enemy quantity band retained stale text: "
            f"{max_quantity_band_white} white pixels"
        )
    if not reason and not injected_types:
        reason = "no guest NPC identity reached the participant list"
    if not reason and len(npc_objects) != 1:
        reason = (
            "x1 guest row constructed "
            f"{len(npc_objects)} guest NPC objects, expected 1"
        )
    if not reason and ("74", "90") not in status_geometry:
        reason = f"enemy HUD backing geometry was {status_geometry}, expected 74x90"
    if not reason and ("76", "40") not in target_icon_geometry:
        reason = (
            f"enemy HUD icon geometry was {target_icon_geometry}, expected 76x40"
        )
    if not reason and not gameplay:
        reason = "gameplay was not reached"
    if not reason and not clean_exit:
        reason = f"process exit was not clean: {process.returncode}"

    report = {
        "schema": 1,
        "passed": passed,
        "reason": reason or "complete guest NPC selector/gameplay lifecycle",
        "executable": str(exe),
        "executable_sha256": hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "captures": captures,
        "traversed_guests": traversed,
        "unique_preview_frames": unique_preview_frames,
        "max_quantity_band_white_pixels": max_quantity_band_white,
        "injected_types": injected_types,
        "npc_objects": npc_objects,
        "expected_npc_objects": 1,
        "hud_status_geometry": status_geometry,
        "hud_target_icon_geometry": target_icon_geometry,
        "gameplay": gameplay,
        "clean_exit": clean_exit,
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "hang_stack": str(hang_stack) if hang_stack else None,
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
