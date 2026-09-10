"""Run only process-local input and native captures; never drive desktop input."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import re
import statistics
import time
from run_map_fidelity_gate import game_process_ids

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--players", type=int, choices=(2, 3, 4), default=4)
    parser.add_argument("--map", default=r"Levels\Bayou.exp")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--polls", type=int, default=3000)
    parser.add_argument("--timeout", type=int, default=120)
    parser.add_argument("--baseline", action="store_true", help="Measure without screenshot captures")
    parser.add_argument("--hud-fixture", action="store_true", help="Equip each player through the native pickup path for icon/ammo visual checks")
    args = parser.parse_args()
    if game_process_ids():
        parser.error("Vigilante82PC is already running; refusing a concurrent native test")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    fixture = output / "input.txt"
    pulses = ["[gameplay]", f"20+{max(300, args.polls)}=P1:CROSS,P2:CROSS"]
    for tick in range(120, args.polls, 240):
        pulses += [f"{tick}+10=P1:LEFT", f"{tick+40}+10=P2:RIGHT"]
    if not args.baseline: pulses.append("350+1=")
    fixture.write_text("\n".join(pulses) + "\n", encoding="utf-8")
    env = os.environ.copy()
    env.update({
        "RECOMPONE_V82_SPLIT_PLAYERS": str(args.players),
        "RECOMPONE_V82_SPLIT_HUD_FIXTURE": "1" if args.hud_fixture else "0",
        "RECOMPONE_V82_SPLIT_MAP": args.map,
        "RECOMPONE_INPUT_FILE": str(fixture),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1",
        "RECOMPONE_FORCE_PAD2_CONNECTED": "1",
        "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_GPU_HLE": "1",
        "RECOMPONE_GRAPHICS_PRESET": "Enhanced",
        "RECOMPONE_MUTE": "1",
        "RECOMPONE_UNTHROTTLED": "1",
        "RECOMPONE_LOG_PATH": str(output / "runtime.log"),
        "RECOMPONE_MOD_DIR": str(ROOT / "V8_2_LOOSE/mods"),
        "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": str(args.polls),
        "RECOMPONE_PRESENTATION_CAPTURE": "1",
        "RECOMPONE_PRESENTATION_RESOLUTION": "1280x720",
        "RECOMPONE_CAPTURE_DIR": str(output),
        "RECOMPONE_CAPTURE_SCRIPTED_STAGE": "gameplay",
        "RECOMPONE_TRACE_WINDOW_FPS": "1",
    })
    if args.baseline:
        env["RECOMPONE_PRESENTATION_CAPTURE"] = "0"
        env["RECOMPONE_CAPTURE_SCRIPTED_STAGE"] = "disabled"
        env["RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES"] = "1"
    started = time.monotonic()
    with (output / "stdout.log").open("w") as stdout, (output / "stderr.log").open("w") as stderr:
        process = subprocess.Popen([str(ROOT / "V8_2_LOOSE/Vigilante82PC.exe"), "--loose", str(ROOT / "V8_2_LOOSE")],
            cwd=ROOT / "V8_2_LOOSE", env=env, stdout=stdout, stderr=stderr,
            creationflags=subprocess.CREATE_NO_WINDOW)
        print(f"Native harness PID={process.pid}", flush=True)
        timed_out = False
        try:
            code = process.wait(timeout=args.timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            process.kill()
            code = process.wait()
    result = dict(exit_code=code, timed_out=timed_out, seconds=round(time.monotonic()-started, 2), players=args.players, bots=6-args.players, map=args.map)
    log = (output / "runtime.log").read_text(encoding="utf-8", errors="replace")
    result["verified_roster"] = f"[V82SplitVerified] combatants=6 humans={args.players} bots={6-args.players}" in log
    # Exclude boot/loading and the first five one-second gameplay samples.
    gameplay = log.split("[V82SplitState] tick=1 ", 1)
    fps = [float(x) for x in re.findall(r"\[HostFpsTitle\] fps=([0-9.]+)", gameplay[-1])] if len(gameplay) == 2 else []
    steady = fps[5:]
    result["fps"] = dict(samples=len(steady), median=round(statistics.median(steady), 2), minimum=round(min(steady), 2), maximum=round(max(steady), 2)) if steady else None
    result["capture_free"] = args.baseline and not any(output.glob("*.ppm"))
    (output / "result.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps(result), flush=True)
    return int(code != 0 or timed_out or not result["verified_roster"])


if __name__ == "__main__":
    raise SystemExit(main())
