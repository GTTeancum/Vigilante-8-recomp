#!/usr/bin/env python3
"""Hidden, silent, image-free acceptance for Y's native selector audio."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import subprocess
import time
from pathlib import Path

from run_reference_soak import capture_managed_stack, stop_process


REPO = Path(__file__).resolve().parents[2]
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_INPUT = (
    Path(__file__).with_name("input-scripts")
    / "native_y_selector_audio_acceptance.txt"
)
DEFAULT_OUTPUT = (
    REPO / "artifacts" / "v82-map-fidelity-current"
    / "y-selector-audio-acceptance"
)
EXIT_AFTER_POLLS = 6500
FATAL_MARKERS = (
    "[fatal]",
    "unhandled exception",
    "unmapped call",
    "out of vram",
    "outofmemoryexception",
)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return ""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--timeout", type=float, default=360.0)
    parser.add_argument("--heartbeat-timeout", type=float, default=45.0)
    args = parser.parse_args()

    loose = args.loose.resolve()
    exe = loose / "Vigilante82PC.exe"
    fixture = args.input.resolve()
    output = args.output.resolve()
    sound_bank = loose / "SHELL" / "SOUNDS.SND"
    for required in (exe, fixture, sound_bank, loose / "SLUS_008.68"):
        if not required.is_file():
            raise FileNotFoundError(required)

    output.mkdir(parents=True, exist_ok=True)
    known_outputs = (
        output / "stdout.log",
        output / "stderr.log",
        output / "runtime.log",
        output / "acceptance.json",
        output / "hang-stack.txt",
    )
    for path in known_outputs:
        if path.is_file():
            path.unlink()
    unexpected_images = tuple(output.glob("*.ppm")) + tuple(output.glob("*.bmp"))
    if unexpected_images:
        raise RuntimeError(
            "image-free output contains stale images: "
            + ", ".join(path.name for path in unexpected_images)
        )

    sound = sound_bank.read_bytes()
    if len(sound) < 4:
        raise ValueError("selector sound bank is truncated")
    sound_entries = struct.unpack_from("<H", sound)[0]
    if sound_entries != 27:
        raise ValueError(
            f"selector sound bank has {sound_entries} entries, expected 27"
        )

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
        "RECOMPONE_PRESENTATION_CAPTURE": "0",
        "RECOMPONE_DISPLAY_PROBE_IMAGES": "0",
        "RECOMPONE_CAPTURE_NATIVE_GUEST_SELECTOR": "0",
        "RECOMPONE_CAPTURE_V82_SELECTOR_SETTLE": "0",
        "RECOMPONE_CAPTURE_V82_SELECTOR_TURNS": "0",
        "RECOMPONE_TRACE_V82_SELECTOR": "1",
        "RECOMPONE_TRACE_V82_SELECTOR_PHYSICS": "1",
        "RECOMPONE_TRACE_V82_RENDER_GROUPS": "1",
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
            cwd=loose,
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
                combined = (
                    read_text(stdout_path)
                    + read_text(stderr_path)
                    + read_text(runtime_path)
                ).lower()
                marker = next(
                    (value for value in FATAL_MARKERS if value in combined),
                    None,
                )
                if marker is not None:
                    reason = f"fatal marker: {marker}"
                    break
                if elapsed > args.timeout:
                    reason = "overall timeout"
                    hang_stack = capture_managed_stack(
                        process, output, "y-selector-audio"
                    )
                    break
                if time.monotonic() - last_progress > args.heartbeat_timeout:
                    reason = "selector heartbeat timeout"
                    hang_stack = capture_managed_stack(
                        process, output, "y-selector-audio"
                    )
                    break
                time.sleep(0.5)
        finally:
            if reason:
                stop_process(process)
            else:
                process.wait(timeout=10)

    # The runtime log is authoritative; desktop builds mirror it to stderr.
    text = read_text(runtime_path) or read_text(stderr_path)
    suppression = re.findall(
        r"\[V82SelectorSound\] suppressed suspension-impact "
        r"stable=guest\.v8\.y_the_alien controller=Flying "
        r"sample=([1-3]) voice=1 caller=0x80106DE0",
        text,
    )
    voice = re.findall(
        r"\[V82SelectionVoice\] guest=12 "
        r"stable=guest\.v8\.y_the_alien sample=26 "
        r"native_voice=3 .* entries=27 ",
        text,
    )
    built = "created guest.v8.y_the_alien native-selector object=" in text
    physics = re.search(
        r"\[V82SelectorPhysics\] guest=12 frame=1[^\r\n]*",
        text,
    )
    contact_supports = (
        len(re.findall(r" w[0-5]=0x[0-9A-Fa-f]+", physics.group(0)))
        if physics is not None
        else 0
    )
    contact_render_packets = len(re.findall(
        r"\[V82RenderGroup\][^\r\n]*"
        r"v82-imported=guest\.v8\.y_the_alien "
        r"bank=selector-transform\b",
        text,
    ))
    collision_only_contact_bank = (
        contact_supports == 4 and contact_render_packets == 0
    )
    accepted = (
        "entered native enemy selector after accepting player_guest=12" in text
    )
    clean_exit = (
        process.returncode == 0
        and f"deterministic replay completed at poll {EXIT_AFTER_POLLS}" in text
    )
    images = [
        str(path)
        for pattern in ("*.ppm", "*.bmp", "*.png")
        for path in output.glob(pattern)
    ]
    passed = (
        not reason
        and len(suppression) == 1
        and len(voice) == 1
        and built
        and collision_only_contact_bank
        and accepted
        and clean_exit
        and not images
    )
    if not reason and len(suppression) != 1:
        reason = f"Y suspension-impact suppressions={suppression}, expected one"
    if not reason and len(voice) != 1:
        reason = f"Y selection-voice events={len(voice)}, expected one"
    if not reason and not built:
        reason = "Y native selector preview was not constructed"
    if not reason and not collision_only_contact_bank:
        reason = (
            "Y selector contact-bank contract failed: "
            f"supports={contact_supports}, "
            f"render-packet groups={contact_render_packets}"
        )
    if not reason and not accepted:
        reason = "Y was not accepted into the native enemy selector"
    if not reason and not clean_exit:
        reason = f"process exit was not clean: {process.returncode}"
    if not reason and images:
        reason = f"image-free run produced images: {images}"

    report = {
        "schema": 1,
        "passed": passed,
        "reason": reason or "Y accepted without crash or suspension impact",
        "executable": str(exe),
        "executable_sha256": hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "sound_bank": str(sound_bank),
        "sound_bank_sha256": hashlib.sha256(sound).hexdigest().upper(),
        "sound_entries": sound_entries,
        "fixture": str(fixture),
        "selector_preview_built": built,
        "contact_supports": contact_supports,
        "contact_bank_render_packet_groups": contact_render_packets,
        "collision_only_contact_bank": collision_only_contact_bank,
        "suspension_impact_suppressions": len(suppression),
        "selection_voice_events": len(voice),
        "entered_enemy_selector": accepted,
        "clean_exit": clean_exit,
        "images": images,
        "process_exit_code": process.returncode,
        "elapsed_seconds": round(time.monotonic() - started, 3),
        "hang_stack": str(hang_stack) if hang_stack else None,
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(report, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
