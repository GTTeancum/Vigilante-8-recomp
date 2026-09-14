"""Exercise all imported campaigns through native menus in one living process.

Uses the explicit result fixture to drive progression after real gameplay.
Never closes or restarts the game, including on a failed assertion or timeout.
"""
import argparse
import base64
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import time

from PIL import Image
from audit_quest_sources import V8_NAMES
from build_loading_cards import ARENAS
from run_map_fidelity_gate import game_process_ids

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"


def script():
    lines = ["540+3=START", "900+3=START"]
    for character in range(12):
        visit = character + 1
        lines += [f"[after:v82_options@{visit}]", "156+3=CROSS", "276+3=DOWN", "336+3=CROSS",
                  f"[after:choose_player@{visit}]"]
        lines += [f"{90+step*50}+3=LEFT" for step in range(12-character)]
        lines += ["750+1=NONE", "850+3=CROSS"]
        for mission in range(4):
            n = character * 4 + mission + 1
            lines += [f"[after:quest_route@{n}]", "360+1=NONE", "480+3=CROSS",
                      f"[after:v82_ready_prompt@{n}]", "120+1=NONE", "210+3=PHYS:RT",
                      f"[after:gameplay@{n}]", "90+1=NONE", "180+3=CROSS",
                      "240+60=PHYS:RT", "450+1=NONE",
                      f"[after:quest_result@{n}]", "360+1=NONE", "480+3=CROSS"]
        lines += [f"[after:quest_ending@{visit}]", "180+1=NONE"]
    return "\n".join(lines)


def checkpoint(log, catalog):
    fixtures = list(re.finditer(r"\[QuestFixture\] result=win type=(\d+) mission=(\d+)", log))
    records = []
    for index, event in enumerate(fixtures):
        character, mission = divmod(index, 4)
        if character >= 12: raise AssertionError("More than 48 mission completions")
        assert (int(event[1]), int(event[2])) == (64+character, mission), event.group()
        authored = catalog[character]["missions"][mission]
        start = log.rfind("[Input] stage 'quest_route'", 0, event.start())
        segment = log[start:event.start()]
        arena = ARENAS[authored["arena"]]
        terrain = arena.removeprefix("LEVELS_").replace("_", "/").lower()
        assert f"active terrain atlas=levels/{terrain} " in segment, (index, arena)
        loading_card_correct = f"selected loading card overlay arena={arena}:" in segment
        actors = re.findall(r"\[QuestActor\] type=(\d+) spawn=(-?\d+) found=0x([0-9A-F]+)", segment)
        expected = [(a["sentinel"] or 64+V8_NAMES.index(a["vehicle"].removeprefix("guest.v8.")), a["spawn"])
                    for a in authored["actors"] if not a["deferred"]]
        assert [(int(t), int(s)) for t,s,p in actors] == expected, (index, actors, expected)
        assert all(int(p,16) for t,s,p in actors), (index, "missing spawn")
        assert f"[QuestPlayer] type={64+character} id=-1" in segment, (index, "player identity")
        records.append({"character": V8_NAMES[character], "mission": mission+1, "arena": arena,
                        "authored_active_actors": len(actors), "gameplay_and_result_trigger": True,
                        "loading_card_correct": loading_card_correct,
                        "resource_errors": re.findall(r"\[V82Fatal\].*", segment)})
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if game_process_ids(): raise RuntimeError("An existing game is open; no process was changed")
    out = args.output.resolve(); out.mkdir(parents=True, exist_ok=False)
    (out/"input.txt").write_text(script())
    catalog = json.loads((LOOSE/"mods/v8_to_v82_guest_roster/quests.json").read_text())["campaigns"]
    settings = LOOSE/"settings.json"; settings_before = settings.read_bytes()
    env = {k:v for k,v in os.environ.items() if not k.startswith("RECOMPONE_")}
    env.update({"RECOMPONE_INPUT_FILE":str(out/"input.txt"), "RECOMPONE_DISABLE_LIVE_INPUT":"1",
                "RECOMPONE_WINDOW_VISIBLE":"0", "SDL_AUDIODRIVER":"dummy", "RECOMPONE_MUTE":"1",
                "RECOMPONE_SUPPRESS_RUMBLE":"1", "RECOMPONE_UNTHROTTLED":"1",
                "RECOMPONE_GPU_HLE":"1", "RECOMPONE_GRAPHICS_PRESET":"Enhanced",
                "RECOMPONE_CAPTURE_DIR":str(out), "RECOMPONE_CAPTURE_SCRIPTED_STAGE":"*",
                "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES":"1", "RECOMPONE_PRESENTATION_CAPTURE":"1",
                "RECOMPONE_PRESENTATION_RESOLUTION":"1280x720", "RECOMPONE_LOG_PATH":str(out/"runtime.log"),
                "RECOMPONE_TRACE_INPUT":"1", "RECOMPONE_TRACE_NATIVE_OPTIONS":"1",
                "RECOMPONE_TRACE_VRAM_PACKING":"1",
                "RECOMPONE_TRACE_V82_SELECTOR":"1", "RECOMPONE_TRACE_QUEST":"1",
                "RECOMPONE_QUEST_TEST_RESULT":"win", "RECOMPONE_V82_QUEST_SAVE_PATH":str(out/"quest-progress.json"),
                "RECOMPONE_MOD_DIR":str(LOOSE/"mods")})
    startup = subprocess.STARTUPINFO(); startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW; startup.wShowWindow=0
    with (out/"stdout.log").open("wb") as stdout, (out/"stderr.log").open("wb") as stderr:
        process = subprocess.Popen([str(LOOSE/"Vigilante82PC.exe"), "--loose", str(LOOSE)], cwd=LOOSE,
                                   env=env, stdout=stdout, stderr=stderr, startupinfo=startup,
                                   creationflags=subprocess.CREATE_NO_WINDOW)
    report = {"pid":process.pid, "exe_sha256":hashlib.sha256((LOOSE/"Vigilante82PC.exe").read_bytes()).hexdigest(),
              "continuous_process":True, "result_fixture":"win after 600 gameplay frames", "complete":False}
    (out/"process.json").write_text(json.dumps(report,indent=2))
    print(f"Continuous Quest run PID {process.pid}; the game will remain open", flush=True)
    started = changed = time.monotonic(); previous = -1; reported_errors = 0
    try:
        while True:
            if process.poll() is not None: raise RuntimeError(f"Game exited unexpectedly: {process.returncode}")
            log = (out/"runtime.log").read_text(errors="replace") if (out/"runtime.log").exists() else ""
            records = checkpoint(log, catalog)
            errors = re.findall(r"\[V82Fatal\].*|\[V82Vehicles\].*backing VRAM rectangle.*already released|\[V82Compat\] PC heap exhausted.*", log)
            for error in errors[reported_errors:]:
                print(f"RESOURCE FAILURE after {len(records)} result triggers: {error}", flush=True)
            reported_errors = len(errors)
            # VRAM exhaustion is recovered by the game, but still recorded as
            # a failed resource check. Continue coverage until an unrecovered fatal.
            fatal = next((e for e in re.finditer(r"\[V82Fatal\].*", log) if "Out of VRAM" not in e.group()), None)
            if fatal: raise RuntimeError(f"{fatal.group()}; game left open")
            if len(records) != previous:
                previous = len(records); changed = time.monotonic()
                report["missions"] = records
                (out/"checkpoint.json").write_text(json.dumps(report,indent=2))
                print(f"{previous}/48 missions reached their result trigger; PID {process.pid}", flush=True)
            # Convert each completed native capture once; retain unique visit labels.
            for ppm in out.glob("recompone_present_*.ppm"):
                if not ppm.with_suffix(".png").exists() and time.time()-ppm.stat().st_mtime > 2:
                    with Image.open(ppm) as im: im.save(ppm.with_suffix(".png"))
            menus = len(re.findall(r"\[Input\] stage 'v82_options'", log))
            if len(records) == 48 and menus == 13:
                save = json.loads((out/"quest-progress.json").read_text())
                assert all(base64.b64decode(save[c["vehicle"]])[0] == 31 for c in catalog)
                endings = re.findall(r"\[CdStream\] end '(MOVIES/V8_[^']+)' .*frames=(\d+) xa=(\d+)", log)
                assert len(endings) == 12 and all(int(frames)>0 for path,frames,xa in endings), endings
                assert [path.casefold() for path,frames,xa in endings] == [
                    campaign["endingMovie"].casefold() for campaign in catalog], endings
                assert len(re.findall(r"\[Input\] stage 'quest_result'",log)) == 48
                releases = re.findall(r"\[V82MatchResources\] released banks=(\d+) pc_allocations=(\d+)", log)
                assert len(releases) == 48 and all(int(banks)>0 for banks,count in releases), releases
                scene_releases = log.count("[V82MatchResources] cleared scene tracking before next load")
                lookup_releases = re.findall(r"\[TexturePack\] released scene lookups=(\d+)", log)
                assert scene_releases == 48 and len(lookup_releases) == 48, (scene_releases, lookup_releases)
                resource_errors = re.findall(r"\[V82Fatal\].*|\[V82Compat\] reclaimed front-end SPU.*|\[V82Vehicles\].*backing VRAM rectangle.*already released|\[V82Compat\] PC heap exhausted.*", log)
                report.update(complete=True, elapsed_seconds=round(time.monotonic()-started,2),
                              endings=endings, all_campaign_progress=31, main_menu_returns=12, game_left_open=True,
                              match_resource_releases=releases, resource_errors=resource_errors,
                              scene_releases=scene_releases, texture_lookup_releases=lookup_releases,
                              passed=not resource_errors and all(r["loading_card_correct"] for r in records))
                break
            if time.monotonic()-changed > 240: raise TimeoutError("No new mission result for four minutes; game left open")
            time.sleep(2)
    except BaseException as error:
        report["error"] = str(error)
        raise
    finally:
        settings.write_bytes(settings_before)
        report["game_still_running"] = process.poll() is None
        (out/"proof.json").write_text(json.dumps(report,indent=2))
    print(json.dumps({k:v for k,v in report.items() if k not in ("missions","endings")}),flush=True)


if __name__ == "__main__": main()
