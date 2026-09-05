"""One hidden, muted native OPTIONS capture. No host input or desktop capture."""
import hashlib
import argparse
import json
import os
import re
from pathlib import Path
import shutil
import subprocess
import time
from PIL import Image
from run_map_fidelity_gate import game_process_ids

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"
OUT = ROOT / "artifacts/transformation-modes-20260904/options"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=OUT)
    args = parser.parse_args()
    out = args.output.resolve()
    if game_process_ids():
        raise RuntimeError("A game is already running; refusing concurrent launch")
    out.mkdir(parents=True, exist_ok=True)
    settings = LOOSE / "settings.json"
    backup = out / "settings.before.json"
    if backup.exists():
        raise RuntimeError("Evidence output already exists; do not overwrite earlier settings backup")
    shutil.copy2(settings, backup)
    exe = LOOSE / "Vigilante82PC.exe"
    digest = hashlib.sha256(exe.read_bytes()).hexdigest()
    env = {k: v for k, v in os.environ.items() if not k.startswith("RECOMPONE_")}
    env.update({
        "RECOMPONE_INPUT_FILE": str(ROOT / "tools/recompone-v8-2/input-scripts/transformation_options.txt"),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_MUTE": "1", "SDL_AUDIODRIVER": "dummy",
        "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_GPU_HLE": "1", "RECOMPONE_GRAPHICS_PRESET": "Enhanced",
        "RECOMPONE_PRESENTATION_CAPTURE": "1",
        "RECOMPONE_PRESENTATION_RESOLUTION": "1920x1080",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE": "v82_options_native_video",
        "RECOMPONE_CAPTURE_DIR": str(out),
        "RECOMPONE_LOG_PATH": str(out / "runtime.log"),
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "3300",
        "RECOMPONE_TRACE_INPUT": "1", "RECOMPONE_TRACE_NATIVE_OPTIONS": "1",
        "RECOMPONE_UNTHROTTLED": "0",
        "RECOMPONE_MOD_DIR": str(LOOSE / "mods"),
    })
    start = time.monotonic()
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    try:
        with (out / "stdout.log").open("wb") as stdout, (out / "stderr.log").open("wb") as stderr:
            process = subprocess.Popen([str(exe), "--loose", str(LOOSE)], cwd=LOOSE,
                env=env, stdout=stdout, stderr=stderr, startupinfo=startup,
                creationflags=subprocess.CREATE_NO_WINDOW)
            try:
                code = process.wait(timeout=180)
            finally:
                if process.poll() is None:
                    process.terminate()
                    process.wait(timeout=10)
        shutil.copy2(settings, out / "settings.after.json")
        # Lossless encoding of the application's own full-resolution output;
        # no desktop capture, resizing, compositing, or visual modification.
        for frame in out.glob("recompone_present_*.ppm"):
            with Image.open(frame) as image:
                image.save(frame.with_suffix(".png"))
        lines = (out / "runtime.log").read_text(errors="replace").splitlines()
        window = re.compile(r"\[EnhancedPerformance\] frames=(\d+)-(\d+).*?effective-fps=([\d.]+)")
        windows = [(i, {"frames": [int(m[1]), int(m[2])], "fps": float(m[3])})
            for i, line in enumerate(lines) if (m := window.search(line))]
        capture_line = next((i for i, line in enumerate(lines)
            if "[Host] captured presentation" in line), None)
        before = [sample for pos, sample in windows if capture_line is not None and pos < capture_line]
        after = [sample for pos, sample in windows if capture_line is not None and pos > capture_line]
        (out / "proof.json").write_text(json.dumps({
            "executable_sha256": digest, "exit_code": code,
            "elapsed_seconds": time.monotonic() - start,
            "capture_point": "v82_options_native_video poll 700 (Gameplay page)",
            "frames": [str(p) for p in out.glob("recompone_present_*.png")],
            "previous_60_frame_window": before[-1] if before else None,
            "capture_or_next_60_frame_window": after[0] if after else None,
            "saved_mode": json.loads(settings.read_text(encoding="utf-8-sig")).get("V82Transformations"),
            "visual_approval": "pending", "gameplay_verified": False,
        }, indent=2))
        print(f"Native OPTIONS run exited {code}; evidence: {out}")
    finally:
        shutil.copy2(backup, settings)


if __name__ == "__main__":
    main()
