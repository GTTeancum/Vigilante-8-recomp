"""Focused source-special gameplay proof using native, process-local input.

Use the map gate's ordinary --output/--player-type/--map-slots arguments.
This evaluates weapons and lifecycle, not route coverage or frame rate.
"""
from pathlib import Path
import importlib.util
import json
import re
import sys


def summarize(output: Path, player: int) -> dict:
    gate = json.loads((output / "acceptance.json").read_text())
    log = (output / "runtime.log").read_text(errors="replace")
    required = ("clean_exit", "no_fatal", "single_game_process", "exact_executable",
                "all_gameplay_visits", "all_native_defeats", "all_shell_visits",
                "deterministic_completion", "no_edge_pool_drops")
    checks = {key: gate["checks"][key] for key in required}
    counts = {event: len(re.findall(r"\[V8SourceSpecial\] " + event + r"\b", log))
              for event in ("fire", "propagate", "hit", "animation-end")}
    if player in (71, 76):
        checks["original_fire"] = counts["fire"] > 0
        checks["original_propagation"] = counts["propagate"] == counts["fire"] * 8
        checks["all_segments_retired"] = counts["animation-end"] == counts["fire"] * 9
        if gate["hold_for_capture"]:
            checks["native_hits"] = counts["hit"] > 0
    elif player == 3:
        checks["stock_does_not_use_original_callbacks"] = counts["fire"] == 0
    result = dict(passed=all(checks.values()), player=player, checks=checks,
                  source_events=counts, executable_sha256=gate["executable_sha256"],
                  captures=gate["captured_images"],
                  scope="source specials and native lifecycle; not a map or FPS acceptance")
    (output / "source_special_acceptance.json").write_text(json.dumps(result, indent=2))
    return result


def main() -> int:
    path = Path(__file__).with_name("run_map_fidelity_gate.py")
    spec = importlib.util.spec_from_file_location("gate", path)
    gate = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(gate)
    original_fixture, original_popen = gate.fixture_text, gate.subprocess.Popen

    def fixture(*args, **kwargs):
        text = original_fixture(*args, **kwargs)
        pulses = "\n".join(f"{poll}+8=L2 # source_special_fire" for poll in (340,420,500,580,660,740))
        return text.replace("[gameplay]\n", "[gameplay]\n" + pulses + "\n")

    def popen(*args, **kwargs):
        env = kwargs.get("env")
        if env is not None and "RECOMPONE_V82_SOAK_WEAPON_AUTOINPUT" in env:
            env = dict(env, RECOMPONE_V82_SOAK_WEAPON_AUTOINPUT="0",
                       RECOMPONE_V82_SOAK_START_KIND="7", RECOMPONE_TRACE_V8_SPECIAL="1")
            if "--hold-for-capture" in sys.argv:
                env.update(RECOMPONE_V82_PROOF_EDGE_VEHICLES="1",
                           RECOMPONE_V82_PROOF_EDGE_VEHICLE_OFFSET="3",
                           RECOMPONE_V82_PROOF_EDGE_VEHICLE_FORWARD="10")
            kwargs["env"] = env
        return original_popen(*args, **kwargs)

    gate.fixture_text, gate.subprocess.Popen = fixture, popen
    if "--verbose-render-log" not in sys.argv:
        sys.argv.append("--verbose-render-log") # Disable map gate's FPS enforcement.
    gate.main()
    output = Path(sys.argv[sys.argv.index("--output") + 1])
    player = int(sys.argv[sys.argv.index("--player-type") + 1])
    result = summarize(output, player)
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
