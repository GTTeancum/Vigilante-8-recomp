"""Native control remapping and map-ready proof; no host input or desktop capture."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from PIL import Image
from run_map_fidelity_gate import game_process_ids
from run_reference_soak import build_input_script

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--ready", action="store_true")
    args = parser.parse_args()
    if game_process_ids():
        raise RuntimeError("A game is already running; refusing concurrent launch")
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    settings = LOOSE / "settings.json"
    before = settings.read_bytes()
    (out / "settings.before.json").write_bytes(before)
    game = json.loads(before.decode("utf-8-sig"))
    # Keep the exact user's Trigger Drive setup as the starting point.
    assert game["InputProfile"] == "Trigger Drive" and game["Pad"]["Cross"] == [101]
    env = {k: v for k, v in os.environ.items() if not k.startswith("RECOMPONE_")}
    fixture = ROOT / "tools/recompone-v8-2/input-scripts/contextual_controls.txt"
    if args.ready:
        # Use the ordinary selector path. Only RT can accept the ready screen;
        # the old fixed-time virtual CROSS press is removed from this fixture.
        script = build_input_script(0).replace("2640+3=CROSS\n", "")
        script += "\n30+1=SELECT\n90+3=PHYS:START\n150+1=SELECT\n"
        script += "\n[v82_ready_prompt]\n30+1=SELECT\n180+3=PHYS:RT\n"
        fixture = out / "input.txt"
        fixture.write_text(script)
    env.update({
        "RECOMPONE_INPUT_FILE": str(fixture), "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0", "RECOMPONE_MUTE": "1", "SDL_AUDIODRIVER": "dummy",
        "RECOMPONE_SUPPRESS_RUMBLE": "1", "RECOMPONE_GPU_HLE": "1",
        "RECOMPONE_GRAPHICS_PRESET": "Enhanced", "RECOMPONE_PRESENTATION_CAPTURE": "1",
        "RECOMPONE_PRESENTATION_RESOLUTION": "1920x1080", "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE": "*" if args.ready else "v82_options_native_controls",
        "RECOMPONE_CAPTURE_DIR": str(out), "RECOMPONE_LOG_PATH": str(out / "runtime.log"),
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "4500" if args.ready else "2800",
        "RECOMPONE_TRACE_INPUT": "1", "RECOMPONE_TRACE_NATIVE_OPTIONS": "1",
        "RECOMPONE_UNTHROTTLED": "1", "RECOMPONE_MOD_DIR": str(LOOSE / "mods"),
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
                code = p.wait(timeout=240)
            finally:
                if p.poll() is None:
                    p.terminate()
                    p.wait(timeout=10)
        (out / "settings.after.json").write_bytes(settings.read_bytes())
        for frame in out.glob("recompone_present_*.ppm"):
            with Image.open(frame) as image:
                image.save(frame.with_suffix(".png"))
        log = (out / "runtime.log").read_text(errors="replace")
        after = json.loads(settings.read_text(encoding="utf-8-sig"))
        result = {"exit_code": code, "exe_sha256": hashlib.sha256((LOOSE / "Vigilante82PC.exe").read_bytes()).hexdigest(),
            "frames": [str(p) for p in out.glob("recompone_present_*.png")],
            "p1_profile": after["InputProfile"], "p2_profile": after.get("InputProfile2"),
            "p1_gas": after["Pad"]["Cross"], "p2_down": after["Pad2"]["Down"]}
        (out / "proof.json").write_text(json.dumps(result, indent=2))
        assert code == 0
        if args.ready:
            assert "stage 'v82_ready_prompt'" in log and "stage 'gameplay'" in log
        else:
            assert after["InputProfile"] == "Custom" and after["Pad"]["Cross"] == [101]
            assert after["InputProfile2"] == "Southpaw" and 109 in after["Pad2"]["Down"]
            assert after["Keys"] == game["Keys"] and after["Keys2"] == game["Keys2"]
        print(f"Controls proof passed: {out}")
    finally:
        settings.write_bytes(before)


if __name__ == "__main__":
    main()
