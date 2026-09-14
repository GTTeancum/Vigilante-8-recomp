"""Normal native Quest menu route, using only process-local synthetic input."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from PIL import Image
from run_map_fidelity_gate import game_process_ids

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--original", type=int, default=0)
    parser.add_argument("--mission", type=int, default=0, choices=range(4))
    parser.add_argument("--result", choices=("win", "lose", "objective-fail"))
    parser.add_argument("--save-from", type=Path)
    parser.add_argument("--audit-carousel", action="store_true")
    parser.add_argument("--trace-vram", action="store_true")
    args = parser.parse_args()
    if game_process_ids(): raise RuntimeError("Game already running")
    assert -1 <= args.original < 12
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    if args.save_from:
        (out / "quest-progress.json").write_bytes(args.save_from.read_bytes())
    elif args.original >= 0 and args.mission:
        import base64
        from audit_quest_sources import V8_NAMES
        data = bytearray(30); data[0] = (1 << (args.mission + 1)) - 1; data[10] = data[20] = 1
        (out / "quest-progress.json").write_text(json.dumps({"guest.v8." + V8_NAMES[args.original]: base64.b64encode(data).decode()}))
    lines = ["540+3=START", "900+3=START", "1080+3=CROSS", "1200+3=DOWN", "1260+3=CROSS", "[choose_player]"]
    if args.audit_carousel:
        assert args.original == -1 and not args.result
        for step in range(27): lines.append(f"{90+step*50}+3=RIGHT")
        lines += ["1500+1=NONE", "1600+3=CROSS"]
    elif args.original >= 0:
        for step in range(12 - args.original): lines.append(f"{90+step*50}+3=LEFT")
    if not args.audit_carousel: lines += ["750+1=NONE", "850+3=CROSS"]
    lines += ["[quest_route]", "360+1=NONE", "480+3=CROSS",
              "[v82_ready_prompt]", "120+1=NONE", "210+3=PHYS:RT", "[gameplay]", "90+1=NONE", "180+3=CROSS", "450+1=NONE",
              "[quest_result]", "360+1=NONE", "480+3=CROSS", "[quest_ending]", "180+1=NONE", "900+3=CROSS"]
    (out / "input.txt").write_text("\n".join(lines))
    settings = LOOSE / "settings.json"
    before = settings.read_bytes()
    env = {k:v for k,v in os.environ.items() if not k.startswith("RECOMPONE_")}
    env.update({"RECOMPONE_INPUT_FILE":str(out / "input.txt"), "RECOMPONE_DISABLE_LIVE_INPUT":"1",
        "RECOMPONE_WINDOW_VISIBLE":"0", "SDL_AUDIODRIVER":"dummy", "RECOMPONE_MUTE":"1",
        "RECOMPONE_SUPPRESS_RUMBLE":"1", "RECOMPONE_UNTHROTTLED":"1",
        "RECOMPONE_GPU_HLE":"1", "RECOMPONE_GRAPHICS_PRESET":"Enhanced",
        "RECOMPONE_CAPTURE_DIR":str(out), "RECOMPONE_CAPTURE_SCRIPTED_STAGE":"*",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES":"1", "RECOMPONE_PRESENTATION_CAPTURE":"1",
        "RECOMPONE_PRESENTATION_RESOLUTION":"1280x720", "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS":"4500",
        "RECOMPONE_LOG_PATH":str(out / "runtime.log"), "RECOMPONE_TRACE_INPUT":"1",
        "RECOMPONE_TRACE_NATIVE_OPTIONS":"1", "RECOMPONE_TRACE_V82_SELECTOR":"1", "RECOMPONE_TRACE_QUEST":"1",
        "RECOMPONE_V82_QUEST_SAVE_PATH":str(out / "quest-progress.json"),
        "RECOMPONE_MOD_DIR":str(LOOSE / "mods")})
    if args.trace_vram:
        env["RECOMPONE_TRACE_VRAM"] = "1"
    if args.result:
        env["RECOMPONE_QUEST_TEST_RESULT"] = args.result
        env["RECOMPONE_SCRIPT_EXIT_AFTER_POLLS"] = "6000"
    if args.audit_carousel:
        env["RECOMPONE_V82_UNLOCK_ROSTER"] = "1"
        env["RECOMPONE_QUEST_TEST_NATIVE_PROGRESS"] = "1"
        env["RECOMPONE_SCRIPT_EXIT_AFTER_POLLS"] = "5500"
    start = subprocess.STARTUPINFO(); start.dwFlags |= subprocess.STARTF_USESHOWWINDOW; start.wShowWindow=0
    try:
        with (out / "stdout.log").open("wb") as stdout, (out / "stderr.log").open("wb") as stderr:
            p = subprocess.Popen([str(LOOSE / "Vigilante82PC.exe"),"--loose",str(LOOSE)],cwd=LOOSE,env=env,
                stdout=stdout,stderr=stderr,startupinfo=start,creationflags=subprocess.CREATE_NO_WINDOW)
            try: code=p.wait(timeout=240)
            finally:
                if p.poll() is None: p.terminate(); p.wait(timeout=10)
        for f in out.glob("recompone_present_*.ppm"):
            with Image.open(f) as im: im.save(f.with_suffix(".png"))
        log=(out / "runtime.log").read_text(errors="replace")
        import re
        actor_spawns = re.findall(r"\[QuestActor\] type=(\d+) spawn=(-?\d+) found=0x([0-9A-F]+)", log)
        result={"exit_code":code,"original_type":args.original,
            "exe_sha256":hashlib.sha256((LOOSE / "Vigilante82PC.exe").read_bytes()).hexdigest(),
            "route_reached":"stage 'quest_route'" in log,"gameplay_reached":"stage 'gameplay'" in log,
            "selected_type_correct": args.original < 0 or f"selected guest type={64 + args.original} " in log,
            "all_actor_spawns_found": bool(actor_spawns) and all(int(a[2],16) != 0 for a in actor_spawns),
            "quest_excludes_y": "to=guest.12" not in log}
        result["player_identity_correct"] = bool(re.search(r"\[QuestPlayer\] type=\d+ id=-1\b", log))
        if args.audit_carousel:
            visited = set(re.findall(r"\[V82SelectorCarousel\].*? to=((?:retail|guest)\.\d+) ", log))
            expected = {f"retail.{i}" for i in range(18) if i not in (5,11,17)} | {f"guest.{i}" for i in range(12)}
            result["carousel_visited"] = sorted(visited)
            result["carousel_complete"] = visited == expected
        if args.result and args.original >= 0:
            import base64
            from audit_quest_sources import V8_NAMES
            saved = json.loads((out / "quest-progress.json").read_text())
            progress = base64.b64decode(saved["guest.v8." + V8_NAMES[args.original]])[0]
            expected = (1 << (args.mission + 1)) - 1
            if args.result != "lose":
                expected |= 1 << (args.mission + 1)
                if args.result == "objective-fail": expected &= ~(1 << args.mission)
            result["progress"] = progress
            result["progress_expected"] = expected
            result["progress_correct"] = progress == expected
            result["mission_correct"] = f"[QuestFixture] result={args.result} type={64 + args.original} mission={args.mission}" in log
            result["ending_reached"] = "stage 'quest_ending'" in log
        (out / "proof.json").write_text(json.dumps(result,indent=2)); print(json.dumps(result))
        assert code==0 and result["route_reached"] and result["gameplay_reached"]
        assert result["selected_type_correct"] and result["all_actor_spawns_found"] and result["quest_excludes_y"]
        assert result["player_identity_correct"]
        if args.audit_carousel: assert result["carousel_complete"]
        if args.result and args.original >= 0:
            assert result["progress_correct"] and result["mission_correct"]
            assert result["ending_reached"] == (args.mission == 3 and args.result == "win")
    finally: settings.write_bytes(before)

if __name__=="__main__": main()
