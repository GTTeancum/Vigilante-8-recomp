"""Native Audio page and mixed-WAV proof with process-local input only."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
from PIL import Image
from run_map_fidelity_gate import game_process_ids

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if game_process_ids():
        raise RuntimeError("A game is already running; refusing concurrent launch")
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    settings = LOOSE / "settings.json"
    before = settings.read_bytes()
    (out / "settings.before.json").write_bytes(before)
    game = json.loads(before.decode("utf-8-sig"))
    game.pop("Soundtrack", None)  # Exercise migration/default, not an override.
    settings.write_text(json.dumps(game, indent=2), encoding="utf-8")
    env = {k: v for k, v in os.environ.items() if not k.startswith("RECOMPONE_")}
    env.update({
        "RECOMPONE_INPUT_FILE": str(ROOT / "tools/recompone-v8-2/input-scripts/soundtrack_options.txt"),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1", "RECOMPONE_WINDOW_VISIBLE": "0",
        "SDL_AUDIODRIVER": "dummy", "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_GPU_HLE": "1", "RECOMPONE_GRAPHICS_PRESET": "Enhanced",
        "RECOMPONE_PRESENTATION_CAPTURE": "1", "RECOMPONE_PRESENTATION_RESOLUTION": "1920x1080",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE": "v82_options_native_video",
        "RECOMPONE_CAPTURE_DIR": str(out), "RECOMPONE_LOG_PATH": str(out / "runtime.log"),
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "2400", "RECOMPONE_TRACE_INPUT": "1",
        "RECOMPONE_TRACE_NATIVE_OPTIONS": "1", "RECOMPONE_TRACE_AUDIO": "1",
        "RECOMPONE_AUDIO_CAPTURE": str(out / "audio.wav"), "RECOMPONE_UNTHROTTLED": "0",
        "RECOMPONE_MOD_DIR": str(LOOSE / "mods"),
    })
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    try:
        with (out / "stdout.log").open("wb") as stdout, (out / "stderr.log").open("wb") as stderr:
            p = subprocess.Popen([str(LOOSE / "Vigilante82PC.exe"), "--loose", str(LOOSE)],
                cwd=LOOSE, env=env, stdout=stdout, stderr=stderr, startupinfo=startup,
                creationflags=subprocess.CREATE_NO_WINDOW)
            try:
                code = p.wait(timeout=180)
            finally:
                if p.poll() is None:
                    p.terminate()
                    p.wait(timeout=10)
        shutil.copy2(settings, out / "settings.after.json")
        for frame in out.glob("recompone_present_*.ppm"):
            with Image.open(frame) as image:
                image.save(frame.with_suffix(".png"))
        log = (out / "runtime.log").read_text(errors="replace")
        (out / "proof.json").write_text(json.dumps({
            "exit_code": code,
            "exe_sha256": hashlib.sha256((LOOSE / "Vigilante82PC.exe").read_bytes()).hexdigest(),
            "saved_soundtrack": json.loads(settings.read_text(encoding="utf-8-sig")).get("Soundtrack"),
            "soundtrack_events": [line for line in log.splitlines() if "[Soundtrack]" in line or "[CDDA] loose" in line],
            "frames": [str(p) for p in out.glob("recompone_present_*.png")],
        }, indent=2))
        assert code == 0, f"game exit {code}"
        assert json.loads(settings.read_text(encoding="utf-8-sig"))["Soundtrack"] == 2
        print(f"Native soundtrack proof: {out}")
    finally:
        settings.write_bytes(before)


if __name__ == "__main__":
    main()
