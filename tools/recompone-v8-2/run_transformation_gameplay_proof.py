"""Native transformation integration fixture; one hidden/silent loose-root game."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time
from PIL import Image
import run_map_fidelity_gate as gate

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"


def review(out):
    report_path = out / "proof.json"
    report = json.loads(report_path.read_text())
    lines = (out / "runtime.log").read_text(errors="replace").splitlines()
    for frame in out.glob("recompone_present*.ppm"):
        with Image.open(frame) as native_image:
            native_image.save(frame.with_suffix(".png"))
    window = re.compile(r"\[EnhancedPerformance\] frames=(\d+)-(\d+).*?effective-fps=([\d.]+)")
    windows = [(i, {"frames": [int(m[1]), int(m[2])], "fps": float(m[3])})
        for i, line in enumerate(lines) if (m := window.search(line))]
    report["individual_proofs"] = []
    for i, line in enumerate(lines):
        if "[Host] captured presentation '" not in line or not any(
            prefix in line for prefix in ("'auto_waterski_", "'water_attachment_")):
            continue
        name = re.search(r"presentation '([^']+)'", line)[1]
        native_frame = int(name.rsplit("_", 1)[1])
        states = [dict(re.findall(r"([\w-]+)=([^ ]+)", event))
            for event in lines[:i] if f"[WaterAttachment] frame={native_frame} vehicle=" in event]
        state = states[-1] if states else None
        preceding = [sample for pos, sample in windows if pos < i]
        following = [sample for pos, sample in windows if pos > i]
        report["individual_proofs"].append({
            "capture_point": name, "native_gameplay_frame": native_frame,
            "native_attachment_state": state,
            "assembled_water_mode": bool(state) and state["mode"] == "2" and state["transition"] == "0",
            "executable_sha256": report["executable_sha256"],
            "previous_60_frame_window": preceding[-1] if preceding else None,
            "capture_or_next_60_frame_window": following[0] if following else None,
            "file": line.split(" to ", 1)[1], "user_visual_approval": "pending"})
    samples = []
    for line in lines:
        if "[TransformationProbe] sample " not in line:
            continue
        fields = dict(re.findall(r"([\w-]+)=([^ ]+)", line))
        samples.append(fields)
    report["water_checks"] = []
    report["fixture_failures"] = [line for line in lines if "fixture-failed" in line or "[Fatal]" in line]
    report["native_diagnostics"] = list(dict.fromkeys(line for line in lines if "[V82Fatal]" in line))
    recoveries = [dict(re.findall(r"([\w-]+)=([^ ]+)", line)) for line in lines
        if "[TransformationProbe] native-recovery " in line]
    report["recovery_checks"] = []
    for event in recoveries:
        later = [s for s in samples if s["generation"] == event["generation"] and
            s["actor"] == "player" and int(s["frame"]) > int(event["frame"])]
        restored = [s for s in later if s["callback"] == s["initial-callback"] and
            s["mode"] == "0" and s["transition"] == "0"]
        report["recovery_checks"].append({
            "event": event, "started_equipped": event["mode"] == "2",
            "first_normal_sample": restored[0] if restored else None,
            "all_later_samples_alive": bool(later) and all(int(s["health"]) > 0 for s in later),
            "note": "Explicit native recovery event; not an uncommanded drowning event."})
    requests = [dict(re.findall(r"([\w-]+)=([^ ]+)", line)) for line in lines
        if "[TransformationProbe] native-request " in line]
    report["native_action_checks"] = {
        "applicable": report["profile"] == "powerups",
        "requests": len(requests),
        "expected_modes": bool(requests) and all(int(s["mode"]) ==
            (int(s["requested"]) if report["mode"] == 0 else 0) for s in requests),
        "powerup_shield_present_only_when_allowed": bool(requests) and all(int(s["shield"]) ==
            (500 if report["mode"] == 0 and int(s["requested"]) != 0 else 0) for s in requests),
        "note": ("Native pickup action exercised, not physical pickup collision."
            if report["profile"] == "powerups" else "Not exercised by this profile; false values above are not test failures.")}
    for generation in range(1, report["generations"] + 1) if samples else ():
        for actor in (("player",) if report["profile"] == "attachment" else ("player", "ai")):
            subset = [s for s in samples if int(s["generation"]) == generation and s["actor"] == actor]
            water = [s for s in subset if 120 <= int(s["frame"]) <= 330]
            land = [s for s in subset if 480 <= int(s["frame"]) <= 600]
            repeated_water = [s for s in subset if 690 <= int(s["frame"]) <= 840]
            repeated_land = [s for s in subset if 990 <= int(s["frame"]) <= 1050]
            report["water_checks"].append({"generation": generation, "actor": actor,
                "water_samples": len(water),
                "water_samples_complete": len(water) == 8,
                "alive_at_all_water_samples": bool(water) and all(int(s["health"]) > 0 for s in water),
                "original_callback_at_water_samples": bool(water) and all(s["callback"] == s["initial-callback"] for s in water),
                "water_movement_at_all_samples": bool(water) and all(s["mode"] == "2" or s["flying"] == "True" for s in water),
                "no_automatic_pickup_shield": bool(water) and all(s["shield"] == "0" for s in water),
                "land_samples": len(land),
                "land_samples_complete": len(land) == 5,
                "normal_mode_at_all_land_samples": bool(land) and all(s["mode"] == "0" and s["transition"] == "0" for s in land),
                "repeat_water_samples": len(repeated_water),
                "repeat_water_alive_assembled": len(repeated_water) == 6 and all(int(s["health"]) > 0 and s["mode"] == "2" and s["transition"] == "0" and s["callback"] == s["initial-callback"] for s in repeated_water),
                "repeat_land_samples": len(repeated_land),
                "repeat_land_alive_retracted": len(repeated_land) == 3 and all(int(s["health"]) > 0 and s["mode"] == "0" and s["transition"] == "0" and s["callback"] == s["initial-callback"] for s in repeated_land),
                "land_fixture_caveat": ("No dry placement: native recovery chooses player destination; AI drives freely."
                    if generation == report.get("respawn_generation", 0) else
                    "Restored original actor pose; AI may have started over water."
                    if report["executable_sha256"].lower().startswith("8ac67c") else
                    "Placed once on terrain sampled above the native water plane; no holding.")})
    report_path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--maps", default="3")
    parser.add_argument("--player", type=int, default=0)
    parser.add_argument("--players", help="Optional comma-separated type per map")
    parser.add_argument("--respawn-generation", type=int, default=0)
    parser.add_argument("--mode", type=int, choices=(0, 1, 2), default=2)
    parser.add_argument("--images", action="store_true")
    parser.add_argument("--camera-cycles", type=int, choices=(0, 1, 2), default=0,
        help="Native process-local SELECT camera cycles before the capture; no host input")
    parser.add_argument("--inspection-camera", action="store_true",
        help="Opt-in native elevated/wider chase viewpoint for attachment inspection")
    parser.add_argument("--inspection-yaw-degrees", type=int, default=0,
        help="Fixture camera yaw relative to vehicle, -180..180; requires --inspection-camera")
    parser.add_argument("--inspection-pitch-degrees", type=int, default=-45,
        help="Fixture camera elevation, -80..-5; requires --inspection-camera")
    parser.add_argument("--attachment-trace", action="store_true")
    parser.add_argument("--water-site-separation-units", type=int, default=0,
        help="Fixture only: choose clear water this far from the nearest site, 0..128 world units")
    parser.add_argument("--packet-trace", action="store_true",
        help="Log native triangle coordinates and ownership at frame150; no extra images")
    parser.add_argument("--attachment-fit", action="store_true", help="Enable the shared fit candidate for this isolated run")
    parser.add_argument("--profile", choices=("water", "powerups", "attachment"), default="water")
    parser.add_argument("--summarize-only", action="store_true")
    args = parser.parse_args()
    if not 0 <= args.water_site_separation_units <= 128:
        parser.error("Water-site separation must be 0..128 world units")
    if not -180 <= args.inspection_yaw_degrees <= 180 or not -80 <= args.inspection_pitch_degrees <= -5:
        parser.error("Inspection yaw must be -180..180 and pitch -80..-5 degrees")
    if args.summarize_only:
        print(json.dumps(review(args.output.resolve()), indent=2))
        return
    slots = [int(s) for s in args.maps.split(",")]
    players = [int(s) for s in args.players.split(",")] if args.players else [args.player] * len(slots)
    if len(players) != len(slots) or any(p < 0 or p > 255 for p in players):
        parser.error("--players must specify one valid type per map")
    maps = tuple(gate.MAPS[s] for s in slots)
    if gate.game_process_ids():
        raise RuntimeError("Game already running; refusing concurrent launch")
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    settings = LOOSE / "settings.json"
    backup = out / "settings.before.json"
    shutil.copy2(settings, backup)
    config = json.loads(settings.read_text(encoding="utf-8-sig"))
    config["V82Transformations"] = args.mode
    # Use the shared native menu/teardown replay without driving/healing actors.
    gate.active_route_pulses = lambda frames: ()
    fixture = out / "input.txt"
    match_frames = 1080 if args.profile == "attachment" else 660
    fixture.write_text(gate.fixture_text(match_frames, maps, capture_poll=150,
        hold_for_capture=True, proof_camera_cycles=args.camera_cycles), encoding="utf-8")
    exe = LOOSE / "Vigilante82PC.exe"
    digest = hashlib.sha256(exe.read_bytes()).hexdigest()
    env = {k: v for k, v in os.environ.items() if not k.startswith("RECOMPONE_")}
    env.update({
        "RECOMPONE_INPUT_FILE": str(fixture),
        "RECOMPONE_DISABLE_LIVE_INPUT": "1", "RECOMPONE_WINDOW_VISIBLE": "0",
        "RECOMPONE_MUTE": "1", "SDL_AUDIODRIVER": "dummy", "RECOMPONE_SUPPRESS_RUMBLE": "1",
        "RECOMPONE_GPU_HLE": "1", "RECOMPONE_GRAPHICS_PRESET": "Enhanced",
        "RECOMPONE_UNTHROTTLED": "0", "RECOMPONE_TRACE_INPUT": "1",
        "RECOMPONE_LOG_PATH": str(out / "runtime.log"), "RECOMPONE_MOD_DIR": str(LOOSE / "mods"),
        "RECOMPONE_V82_SOAK": "1", "RECOMPONE_V82_UNLOCK_ROSTER": "1",
        "RECOMPONE_V82_SOAK_POWERUPS": "0", "RECOMPONE_V82_SOAK_WEAPONS": "0",
        "RECOMPONE_V82_SOAK_NO_AUTOINPUT": "1", "RECOMPONE_SOAK_TEARDOWN_FRAMES": "0",
        "RECOMPONE_V82_TEST_DEFEAT_FRAME": str(match_frames),
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE": "shell_transition",
        "RECOMPONE_SCRIPT_EXIT_AFTER_STAGE_VISITS": str(len(maps) + 1),
        "RECOMPONE_V82_ARENA_SLOT_SEQUENCE": args.maps,
        "RECOMPONE_V82_PLAYER_TYPE_SEQUENCE": ",".join(str(p) for p in players),
        "RECOMPONE_V82_TRANSFORMATION_PROBE": "1",
        "RECOMPONE_V82_WATER_ATTACHMENT_TRACE": "1" if args.attachment_trace else "0",
        "RECOMPONE_V82_WATER_SITE_SEPARATION_UNITS": str(args.water_site_separation_units),
        "RECOMPONE_V82_WATER_ATTACHMENT_FIT": "1" if args.attachment_fit else "0",
        "RECOMPONE_V82_WATER_INSPECTION_CAMERA": "1" if args.inspection_camera else "0",
        "RECOMPONE_V82_WATER_INSPECTION_YAW_DEGREES": str(args.inspection_yaw_degrees),
        "RECOMPONE_V82_WATER_INSPECTION_PITCH_DEGREES": str(args.inspection_pitch_degrees),
        "RECOMPONE_V82_TRANSFORMATION_PROBE_PROFILE": args.profile,
        "RECOMPONE_V82_TRANSFORMATION_PROBE_RESPAWN_GENERATION": str(args.respawn_generation),
        "RECOMPONE_V82_TRANSFORMATION_PROBE_IMAGES": "1" if args.images else "0",
        "RECOMPONE_PRESENTATION_CAPTURE": "1" if args.images else "0",
        "RECOMPONE_PRESENTATION_RESOLUTION": "1920x1080",
        "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
        "RECOMPONE_CAPTURE_DIR": str(out),
    })
    if args.packet_trace:
        env["RECOMPONE_TRACE_GAMEPLAY_TICKS"] = "150"
        env["RECOMPONE_TRACE_WATER_NEAR_GEOMETRY"] = "1"
    started = time.monotonic()
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    process = None
    try:
        settings.write_text(json.dumps(config, indent=2), encoding="utf-8")
        with (out / "stdout.log").open("wb") as stdout, (out / "stderr.log").open("wb") as stderr:
            process = subprocess.Popen([str(exe), "--loose", str(LOOSE)], cwd=LOOSE,
                env=env, stdout=stdout, stderr=stderr, startupinfo=startup,
                creationflags=subprocess.CREATE_NO_WINDOW)
            while process.poll() is None:
                if time.monotonic() - started > 360 * len(maps):
                    raise TimeoutError("Native transformation integration timed out")
                other_pids = [pid for pid in gate.game_process_ids() if pid != process.pid]
                if other_pids:
                    raise RuntimeError(f"Other game PID(s) {other_pids} appeared; stopping only fixture PID {process.pid}")
                time.sleep(3)
        log = (out / "runtime.log").read_text(errors="replace")
        records = [line for line in log.splitlines() if "[TransformationProbe]" in line or "[V82AutoWaterski]" in line]
        (out / "telemetry.txt").write_text("\n".join(records), encoding="utf-8")
        # Retain native outcomes; no automated visual pass or map certification.
        report = {"executable_sha256": digest, "exit_code": process.returncode,
            "packet_trace": args.packet_trace,
            "water_site_separation_units": args.water_site_separation_units,
            "attachment_fit": args.attachment_fit,
            "camera_cycles": args.camera_cycles,
            "inspection_camera": args.inspection_camera,
            "inspection_yaw_degrees": args.inspection_yaw_degrees,
            "inspection_pitch_degrees": args.inspection_pitch_degrees,
            "maps": slots, "mode": args.mode, "players": players, "profile": args.profile,
            "respawn_generation": args.respawn_generation,
            "elapsed_seconds": time.monotonic() - started,
            "generations": len(re.findall(r"\[TransformationProbe\] begin", log)),
            "water_requests": len(re.findall(r"reason=water-approach requested=2 actual=2", log)),
            "land_requests": len(re.findall(r"reason=dry-ground requested=0 actual=0", log)),
            "visual_approval": "pending", "physics_review": "pending",
            "captures": [str(p) for p in out.glob("recompone_present*.ppm")],
            "performance": gate.performance_summary(log)}
        (out / "proof.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        report = review(out)
        print(json.dumps(report, indent=2))
    except Exception as error:
        # Preserve evidence even when the concurrency/timeout guard interrupts
        # a run. Never turn an interrupted fixture into a clean-exit result.
        if process is not None and process.poll() is None:
            process.terminate()
            process.wait(timeout=10)
        log_path = out / "runtime.log"
        if log_path.exists() and not (out / "proof.json").exists():
            log = log_path.read_text(errors="replace")
            report = {"executable_sha256": digest, "exit_code": None,
                "fixture_pid": process.pid if process else None,
                "outcome": f"Interrupted: {error}", "attachment_fit": args.attachment_fit,
                "camera_cycles": args.camera_cycles,
                "inspection_camera": args.inspection_camera,
                "inspection_yaw_degrees": args.inspection_yaw_degrees,
                "inspection_pitch_degrees": args.inspection_pitch_degrees,
                "maps": slots, "mode": args.mode, "players": players, "profile": args.profile,
                "respawn_generation": args.respawn_generation,
                "generations": len(re.findall(r"\[TransformationProbe\] begin", log)),
                "elapsed_seconds": time.monotonic() - started,
                "visual_approval": "pending", "physics_review": "partial; interrupted",
                "captures": [str(p) for p in out.glob("recompone_present*.ppm")],
                "performance": gate.performance_summary(log)}
            (out / "proof.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
            review(out)
        raise
    finally:
        if process is not None and process.poll() is None:
            process.terminate()
            process.wait(timeout=10)
        shutil.copy2(backup, settings)


if __name__ == "__main__":
    main()
