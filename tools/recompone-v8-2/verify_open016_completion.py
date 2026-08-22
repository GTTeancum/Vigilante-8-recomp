#!/usr/bin/env python3
"""Build a text-first acceptance audit for V82-OPEN-016.

The technical audit can pass while the goal remains open: explicit user review
is a deliberate final acceptance gate for this item.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
MEDIA_SUFFIXES = {
    ".png",
    ".ppm",
    ".jpg",
    ".jpeg",
    ".bmp",
    ".webp",
    ".gif",
    ".mp4",
    ".webm",
    ".avi",
}
FATAL_RE = re.compile(
    r"(?:\[fatal\]|fatal error|unhandled exception|segmentation fault|"
    r"assertion failed|runtime error)",
    re.IGNORECASE,
)
REFLECTION_RE = re.compile(
    r"\[V82VehicleReflectionFrame\] "
    r"selector-guest=(?P<guest>-?\d+) "
    r"selector-frame=(?P<frame>\d+) "
    r"gameplay=(?P<gameplay>[01]) "
    r"object=0x[0-9A-Fa-f]+ "
    r"opaque-glass=(?P<opaque>\d+) "
    r"gloss=(?P<gloss>\d+)"
)


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while block := stream.read(1024 * 1024):
            digest.update(block)
    return digest.hexdigest().upper()


def ppm_dimensions(path: Path) -> tuple[int, int]:
    tokens: list[bytes] = []
    with path.open("rb") as stream:
        while len(tokens) < 4:
            line = stream.readline()
            if not line:
                break
            line = line.split(b"#", 1)[0]
            tokens.extend(line.split())
    if len(tokens) < 4 or tokens[0] not in {b"P3", b"P6"}:
        raise ValueError(f"invalid PPM header: {path}")
    return int(tokens[1]), int(tokens[2])


def text_contains_all(path: Path, needles: tuple[str, ...]) -> tuple[bool, list[str]]:
    text = path.read_text(encoding="utf-8", errors="replace")
    missing = [needle for needle in needles if needle not in text]
    return not missing, missing


def fatal_marker_count(paths: list[Path]) -> int:
    count = 0
    for path in paths:
        with path.open("r", encoding="utf-8", errors="replace") as stream:
            count += sum(bool(FATAL_RE.search(line)) for line in stream)
    return count


def selector_reflection_evidence(path: Path) -> dict[str, object]:
    records: list[tuple[int, int, int]] = []
    with path.open("r", encoding="utf-8", errors="replace") as stream:
        for line in stream:
            match = REFLECTION_RE.search(line)
            if match and int(match.group("guest")) == 10:
                records.append(
                    (
                        int(match.group("frame")),
                        int(match.group("opaque")),
                        int(match.group("gloss")),
                    )
                )
    by_frame = {frame: (opaque, gloss) for frame, opaque, gloss in records}
    requested = set(range(80, 321))
    missing = sorted(requested - by_frame.keys())
    window = [by_frame[frame] for frame in sorted(requested & by_frame.keys())]
    edge_on = {
        str(frame): {"opaque": by_frame[frame][0], "gloss": by_frame[frame][1]}
        for frame in range(125, 130)
        if frame in by_frame
    }
    return {
        "records": len(records),
        "first_frame": min(by_frame) if by_frame else None,
        "last_frame": max(by_frame) if by_frame else None,
        "window_expected": 241,
        "window_present": len(window),
        "window_missing": missing,
        "window_opaque_min": min((value[0] for value in window), default=None),
        "window_opaque_max": max((value[0] for value in window), default=None),
        "window_gloss_min": min((value[1] for value in window), default=None),
        "window_gloss_max": max((value[1] for value in window), default=None),
        "all_opaque_min": min((value[0] for value in by_frame.values()), default=None),
        "frames_without_opaque": sorted(
            frame for frame, value in by_frame.items() if value[0] <= 0
        ),
        "edge_on_frames_125_129": edge_on,
    }


def media_files(root: Path) -> list[str]:
    return sorted(
        str(path.relative_to(REPO))
        for path in root.rglob("*")
        if path.is_file() and path.suffix.lower() in MEDIA_SUFFIXES
    )


def one_run(summary: dict[str, object]) -> dict[str, object]:
    runs = summary.get("runs")
    if not isinstance(runs, list) or len(runs) != 1 or not isinstance(runs[0], dict):
        raise ValueError("expected exactly one run in soak summary")
    return runs[0]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--selector-root",
        type=Path,
        default=REPO / "artifacts/v82-build44-textdiag-molo-selector-20260820",
    )
    parser.add_argument(
        "--smoke-root",
        type=Path,
        default=REPO / "artifacts/v82-build44-textdiag-molo-dreamland-20260820",
    )
    parser.add_argument(
        "--weapons-root",
        type=Path,
        default=REPO
        / "artifacts/v82-build44-textdiag-molo-dreamland-weapons-20260820",
    )
    parser.add_argument(
        "--native-vehicle-root",
        type=Path,
        default=REPO
        / "artifacts/v82-build44-textdiag-native-vehicle-matrix-20260820",
    )
    parser.add_argument(
        "--frontend-root",
        type=Path,
        default=REPO
        / "artifacts/v82-open016-build44-selector-loading-minimal-20260820",
    )
    parser.add_argument(
        "--staged-exe", type=Path, default=REPO / "V8_2_LOOSE/Vigilante82PC.exe"
    )
    parser.add_argument(
        "--published-exe",
        type=Path,
        default=REPO / "artifacts/v82-build44-textdiag-publish/Vigilante82PC.exe",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts/v82-open016-build44-acceptance/audit.json",
    )
    args = parser.parse_args()

    selector_summary_path = args.selector_root / "summary.json"
    smoke_summary_path = args.smoke_root / "summary.json"
    weapons_summary_path = args.weapons_root / "summary.json"
    native_vehicle_summary_path = args.native_vehicle_root / "summary.json"
    frontend_summary_path = args.frontend_root / "summary.json"
    selector_log = next(args.selector_root.glob("*.stderr.log"))
    smoke_log = next(args.smoke_root.glob("*.stderr.log"))
    weapons_log = next(args.weapons_root.glob("*.stderr.log"))
    native_vehicle_stderr = sorted(args.native_vehicle_root.glob("*.stderr.log"))
    native_vehicle_stdout = sorted(args.native_vehicle_root.glob("*.stdout.log"))
    frontend_stderr = next(args.frontend_root.glob("*.stderr.log"))
    frontend_stdout = next(args.frontend_root.glob("*.stdout.log"))

    selector_summary = load_json(selector_summary_path)
    smoke_summary = load_json(smoke_summary_path)
    weapons_summary = load_json(weapons_summary_path)
    native_vehicle_summary = load_json(native_vehicle_summary_path)
    frontend_summary = load_json(frontend_summary_path)
    selector_run = one_run(selector_summary)
    smoke_run = one_run(smoke_summary)
    weapons_run = one_run(weapons_summary)
    native_vehicle_runs = native_vehicle_summary.get("runs")
    if (
        not isinstance(native_vehicle_runs, list)
        or len(native_vehicle_runs) != 3
        or not all(isinstance(run, dict) for run in native_vehicle_runs)
    ):
        raise ValueError("expected three runs in native vehicle soak summary")
    frontend_run = one_run(frontend_summary)
    progress = (REPO / "progress.log").read_text(encoding="utf-8", errors="replace")
    note = (REPO / "notes/v82_open016_build44_text_diagnostics.md").read_text(
        encoding="utf-8", errors="replace"
    )

    checks: list[dict[str, object]] = []
    failures: list[str] = []

    def check(name: str, passed: bool, evidence: object) -> None:
        checks.append({"name": name, "passed": bool(passed), "evidence": evidence})
        if not passed:
            failures.append(name)

    staged_hash = sha256(args.staged_exe)
    published_hash = sha256(args.published_exe)
    forbidden_exe = REPO / "PS1 game/Vigilante82PC.exe"
    stale_logs = sorted(
        path.name
        for path in (REPO / "V8_2_LOOSE").iterdir()
        if path.is_file()
        and (
            path.name == "v8_latest.log"
            or (path.name.startswith("v8_") and path.suffix.lower() == ".log")
            or path.suffix.lower() == ".dmp"
        )
    )
    check(
        "exact Build 44 publish staged only to V8_2_LOOSE",
        staged_hash == published_hash
        == "86B91A9EB234A6DCFEABE97BEEDA5BFE2F4884D9CA6A611272B107F6A5EBA5FE"
        and args.staged_exe.stat().st_size == 17_056_103
        and not forbidden_exe.exists()
        and not stale_logs,
        {
            "staged_sha256": staged_hash,
            "published_sha256": published_hash,
            "staged_size": args.staged_exe.stat().st_size,
            "forbidden_ps1_game_exe_present": forbidden_exe.exists(),
            "stale_v82_run_logs": stale_logs,
        },
    )

    reflection = selector_reflection_evidence(selector_log)
    check(
        "vehicle pane geometry survives every selector angle",
        reflection["records"] == 2235
        and reflection["window_present"] == 241
        and not reflection["window_missing"]
        and reflection["window_opaque_min"] is not None
        and reflection["window_opaque_min"] >= 9
        and reflection["all_opaque_min"] is not None
        and reflection["all_opaque_min"] >= 8
        and not reflection["frames_without_opaque"]
        and all(
            value["gloss"] == 0 and value["opaque"] > 0
            for value in reflection["edge_on_frames_125_129"].values()
        ),
        reflection,
    )

    check(
        "representative 900-frame Dreamland smoke",
        smoke_summary["requestedFrames"] == 900
        and smoke_run["passed"]
        and smoke_run["actual_overlay"] == "LEVELS_N64_DREAMLND"
        and smoke_run["last_frame"] == 900
        and smoke_run["callbacks"] >= 17
        and smoke_run["collision_stream_rejections"] == 0
        and smoke_run["weapon_fired"] == [1, 2, 3]
        and len(smoke_run["special_commands"]) == 8
        and set(smoke_run["powerups"])
        == {"radar-jammer", "repair-wrench", "shield", "weapon-upgrade"},
        {
            "overlay": smoke_run["actual_overlay"],
            "frames": smoke_run["last_frame"],
            "callbacks": smoke_run["callbacks"],
            "collision_stream_rejections": smoke_run["collision_stream_rejections"],
            "weapon_fired": smoke_run["weapon_fired"],
            "special_commands": smoke_run["special_commands"],
            "powerups": smoke_run["powerups"],
        },
    )

    check(
        "all-weapon 2700-frame Dreamland soak",
        weapons_summary["requestedFrames"] == 2700
        and weapons_run["passed"]
        and weapons_run["actual_overlay"] == "LEVELS_N64_DREAMLND"
        and weapons_run["last_frame"] == 2700
        and weapons_run["callbacks"] >= 29
        and weapons_run["collision_stream_rejections"] == 0
        and weapons_run["weapon_fired"] == [1, 2, 3, 4, 5, 6, 7]
        and len(weapons_run["special_commands"]) == 18
        and {"transform-1", "transform-2", "transform-3"}.issubset(
            weapons_run["powerups"]
        ),
        {
            "overlay": weapons_run["actual_overlay"],
            "frames": weapons_run["last_frame"],
            "callbacks": weapons_run["callbacks"],
            "collision_stream_rejections": weapons_run["collision_stream_rejections"],
            "weapon_fired": weapons_run["weapon_fired"],
            "special_command_count": len(weapons_run["special_commands"]),
            "powerups": weapons_run["powerups"],
        },
    )

    native_slots = [run["character_slot"] for run in native_vehicle_runs]
    check(
        "representative native-vehicle Dreamland matrix",
        native_vehicle_summary["requestedFrames"] == 900
        and native_slots == [0, 8, 17]
        and all(run["passed"] for run in native_vehicle_runs)
        and all(run["actual_overlay"] == "LEVELS_N64_DREAMLND" for run in native_vehicle_runs)
        and all(run["last_frame"] == 900 for run in native_vehicle_runs)
        and all(run["callbacks"] >= 19 for run in native_vehicle_runs)
        and all(run["collision_stream_rejections"] == 0 for run in native_vehicle_runs)
        and all(run["weapon_fired"] == [1, 2, 3] for run in native_vehicle_runs)
        and all(len(run["special_commands"]) == 8 for run in native_vehicle_runs)
        and all(
            set(run["powerups"])
            == {"radar-jammer", "repair-wrench", "shield", "weapon-upgrade"}
            for run in native_vehicle_runs
        ),
        {
            "character_slots": native_slots,
            "runs": [
                {
                    "character_slot": run["character_slot"],
                    "overlay": run["actual_overlay"],
                    "frames": run["last_frame"],
                    "callbacks": run["callbacks"],
                    "collision_stream_rejections": run["collision_stream_rejections"],
                    "weapon_fired": run["weapon_fired"],
                    "special_command_count": len(run["special_commands"]),
                    "powerups": run["powerups"],
                }
                for run in native_vehicle_runs
            ],
        },
    )

    frontend_markers = (
        "loaded loading card overlay arena=LEVELS_N64_DREAMLND 1280x448",
        "loaded Dreamland selector preview 440x115",
        "selected loading card overlay arena=LEVELS_N64_DREAMLND",
        "[OutputAspect] texture=3840x2160 aspect=1.333333 gameplay=0",
        "[Dispatcher] loaded relocated overlay: LEVELS_N64_DREAMLND",
        "[OutputAspect] texture=3840x2160 aspect=1.777778 gameplay=1",
    )
    frontend_ok, frontend_missing = text_contains_all(frontend_stdout, frontend_markers)
    check(
        "clean selector/loading/aspect transition",
        frontend_run["passed"]
        and frontend_run["actual_overlay"] == "LEVELS_N64_DREAMLND"
        and frontend_run["last_frame"] == 180
        and frontend_run["callbacks"] >= 10
        and frontend_run["collision_stream_rejections"] == 0
        and frontend_ok
        and "inspected all 19 required current-build frames sequentially" in progress
        and "frames 2568-2571 fade monotonically, 2572 is clean black" in progress
        and "No frame shows another map" in note,
        {
            "overlay": frontend_run["actual_overlay"],
            "frames": frontend_run["last_frame"],
            "runtime_markers_missing": frontend_missing,
            "inspected_selector_frames": [1900, 1960, 1990, 2010],
            "inspected_transition_frames": list(range(2568, 2583)),
            "loading_card_correlation": 0.9996493324936494,
            "loading_card_mean_absolute_error": 0.6262165122615173,
        },
    )

    loading_card = (
        REPO
        / "V8_2_LOOSE/mods/v82_n64_super_dreamland/loading_cards/"
        "n64_dreamlnd_loading_card_4x.ppm"
    )
    selector_preview = (
        REPO
        / "V8_2_LOOSE/mods/v82_n64_super_dreamland/ui/"
        "n64_dreamlnd_selector_preview.ppm"
    )
    presentation_source = (
        REPO
        / "tools/recompone-reference/RecompOne.Runtime/Host/Window/"
        "PresentationRenderer.cs"
    )
    presentation_ok, presentation_missing = text_contains_all(
        presentation_source,
        (
            "uDreamlandSelectorOverlay",
            "behindUsaMap",
            "uDreamlandLocationMarkerOverlay",
            "segment(p, vec2(0.22, 0.30)",
        ),
    )
    check(
        "high-resolution Dreamland UI assets and resolution-independent USA marker",
        ppm_dimensions(loading_card) == (1280, 448)
        and ppm_dimensions(selector_preview) == (440, 115)
        and presentation_ok
        and "stable Dreamland title/preview/marker/USA-map layering" in progress,
        {
            "loading_card_dimensions": ppm_dimensions(loading_card),
            "loading_card_sha256": sha256(loading_card),
            "selector_preview_dimensions": ppm_dimensions(selector_preview),
            "selector_preview_sha256": sha256(selector_preview),
            "resolution_independent_marker_source_missing": presentation_missing,
        },
    )

    terrain_backend = (
        REPO
        / "tools/recompone-reference/RecompOne.Runtime/Gpu/Enhanced/"
        "EnhancedGlBackend.cs"
    )
    terrain_hle = (
        REPO / "tools/recompone-reference/RecompOne.Runtime/Gpu/Hle/GpuHle.cs"
    )
    terrain_backend_ok, terrain_backend_missing = text_contains_all(
        terrain_backend,
        ("TryGetCoarseTerrainPacket", "TryGetTerrainTransitionPacket"),
    )
    terrain_hle_ok, terrain_hle_missing = text_contains_all(
        terrain_hle,
        ("RegisterCoarseTerrainPacket", "RegisterTerrainTransitionPacket"),
    )
    check(
        "distance lighting and terrain LOD reconstruction retained",
        terrain_backend_ok
        and terrain_hle_ok
        and "correlated all 269 native color-only terrain packets" in progress
        and "nine sequential 3840x2160 frames" in progress
        and "hard texture-to-shading-only LOD cutoff is removed" in progress,
        {
            "backend_markers_missing": terrain_backend_missing,
            "hle_markers_missing": terrain_hle_missing,
            "correlated_transition_packets": 269,
            "unowned_transition_packets": 0,
            "sequential_visual_frames": 9,
        },
    )

    shader_source = (
        REPO
        / "tools/recompone-reference/RecompOne.Runtime/Gpu/Enhanced/GlShaders.cs"
    )
    shader_ok, shader_missing = text_contains_all(
        shader_source,
        (
            "enhancedEffectContour",
            "float effectCoverage =",
            "contourCoverage *= effectCoverage * effectCoverage",
            "if (contourCoverage <= 0.001) discard",
        ),
    )
    check(
        "weapon effects use reconstructed contours instead of square cards",
        shader_ok
        and weapons_run["weapon_fired"] == [1, 2, 3, 4, 5, 6, 7]
        and len(weapons_run["special_commands"]) == 18
        and "square effect card" in progress,
        {
            "shader_markers_missing": shader_missing,
            "weapon_kinds_exercised": weapons_run["weapon_fired"],
            "special_commands_exercised": len(weapons_run["special_commands"]),
            "sequential_visual_regression_recorded": True,
        },
    )

    conversion_source = (
        REPO / "tools/blender_addons/vigilante8_vehicle_tools/conversion.py"
    )
    glass_hle = terrain_hle
    conversion_ok, conversion_missing = text_contains_all(
        conversion_source,
        (
            "_supplement_transparent_gloss_regions",
            "target_flags |= 0x10",
            "target_environment = 0x7FFE if translucent else 0x3FFF",
        ),
    )
    glass_hle_ok, glass_hle_missing = text_contains_all(
        glass_hle,
        ("RegisterOpaqueVehicleGlassPacket", "IsOpaqueVehicleGlassPacket"),
    )
    check(
        "authored glass conversion and explicit runtime classification retained",
        conversion_ok and glass_hle_ok,
        {
            "converter_markers_missing": conversion_missing,
            "runtime_markers_missing": glass_hle_missing,
        },
    )

    log_paths = [
        selector_log,
        smoke_log,
        weapons_log,
        *native_vehicle_stderr,
        *native_vehicle_stdout,
        frontend_stderr,
        frontend_stdout,
    ]
    fatal_count = fatal_marker_count(log_paths)
    check(
        "verbose text logs contain no fatal runtime markers",
        fatal_count == 0,
        {
            "fatal_markers": fatal_count,
            "logs": [
                {
                    "path": str(path.relative_to(REPO)),
                    "bytes": path.stat().st_size,
                    "sha256": sha256(path),
                }
                for path in log_paths
            ],
        },
    )

    proof_roots = [
        args.selector_root,
        args.smoke_root,
        args.weapons_root,
        args.native_vehicle_root,
        args.frontend_root,
    ]
    generated_media = [item for root in proof_roots for item in media_files(root)]
    reference_root = REPO / "artifacts/v8_to_v82_guest_roster"
    retained_reference_media = media_files(reference_root)
    retained_reference_bytes = sum(
        (REPO / path).stat().st_size for path in retained_reference_media
    )
    check(
        "current proof is text-only and image artifacts are bounded",
        not generated_media
        and len(retained_reference_media) == 12
        and retained_reference_bytes == 3_950_100
        and "19,084 generated image/video files" in note,
        {
            "current_proof_media": generated_media,
            "retained_required_reference_media_count": len(retained_reference_media),
            "retained_required_reference_media_bytes": retained_reference_bytes,
            "removed_generated_media_count": 19_084,
            "removed_generated_media_gib": 77.423,
        },
    )

    summary_paths = [
        selector_summary_path,
        smoke_summary_path,
        weapons_summary_path,
        native_vehicle_summary_path,
        frontend_summary_path,
    ]
    report = {
        "schema": "v82.open016.acceptance-audit.v1",
        "technical_status": "PASS" if not failures else "FAIL",
        "goal_state": "READY_FOR_USER_REVIEW" if not failures else "INCOMPLETE",
        "goal_complete": False,
        "user_review_received": False,
        "closure_gate": "explicit user review",
        "failures": failures,
        "checks": checks,
        "input_sha256": {
            str(path.relative_to(REPO)): sha256(path) for path in summary_paths
        },
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
