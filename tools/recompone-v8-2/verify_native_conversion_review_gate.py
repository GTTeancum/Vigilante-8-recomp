#!/usr/bin/env python3
"""Verify the exception-free V8:2 conversion review gate from current state."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
ARTIFACTS = ROOT / "artifacts" / "v82-native-conversion"
FINAL_RUN = ARTIFACTS / "final-dreamland-molo-2700"
CURRENT_WATER_ARTIFACTS = ROOT / "artifacts" / "v82-item4-water-reconversion"
CURRENT_DREAMLAND_RUN = (
    CURRENT_WATER_ARTIFACTS / "visible-pass-dreamland-text-soak"
)
CURRENT_DREAMLAND_DIAGNOSTICS = (
    CURRENT_WATER_ARTIFACTS / "visible-pass-dreamland-diagnostic-analysis.json"
)
STOCK_RUN = ARTIFACTS / "item3-stock-v82-reflection-control-current"
ROSTER_RUN = ARTIFACTS / "item3-full-roster-churn-exact-staged"
CONVERTED_MATERIAL_RUN = (
    ARTIFACTS / "item3-molo-material-native-semantics-2"
)
STAGED_EXE = ROOT / "V8_2_LOOSE" / "Vigilante82PC.exe"
PUBLISHED_EXE = (
    ROOT / "tools/recompone-v8-2/reference-host/bin/Release/net10.0/"
    "win-x64/publish/Vigilante82PC.exe"
)
MEDIA = {".ppm", ".png", ".jpg", ".jpeg", ".bmp", ".webp", ".gif", ".mp4"}


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def run_diagnostics_text(run_record: dict[str, object]) -> str:
    chunks: list[str] = []
    for key in ("stdout_log", "stderr_log", "runtime_log"):
        value = run_record.get(key)
        if not isinstance(value, str):
            continue
        try:
            chunks.append(Path(value).read_text(
                encoding="utf-8", errors="replace"
            ))
        except FileNotFoundError:
            pass
    return "\n".join(chunks)


def run(command: list[str]) -> dict[str, object]:
    result = subprocess.run(
        command, cwd=ROOT, text=True, capture_output=True, check=False
    )
    return {
        "command": command,
        "exitCode": result.returncode,
        "stdoutTail": result.stdout.splitlines()[-20:],
        "stderrTail": result.stderr.splitlines()[-20:],
    }


def walk_files_without_reparse_points(
    root: Path,
) -> tuple[list[Path], list[Path]]:
    """Return real files below root without entering junctions or symlinks."""
    files: list[Path] = []
    skipped: list[Path] = []
    reparse_flag = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        current = Path(dirpath)
        retained_dirs: list[str] = []
        for dirname in dirnames:
            candidate = current / dirname
            attributes = getattr(
                candidate.stat(follow_symlinks=False), "st_file_attributes", 0
            )
            if candidate.is_symlink() or attributes & reparse_flag:
                skipped.append(candidate)
            else:
                retained_dirs.append(dirname)
        dirnames[:] = retained_dirs
        files.extend(current / filename for filename in filenames)
    return files, skipped


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output", type=Path,
        default=ARTIFACTS / "review-gate-audit.json",
    )
    args = parser.parse_args()

    checks: dict[str, bool] = {}
    evidence: dict[str, object] = {}

    def record(name: str, passed: bool, value: object) -> None:
        checks[name] = bool(passed)
        evidence[name] = value

    staged_hash = digest(STAGED_EXE)
    published_hash = digest(PUBLISHED_EXE)
    record(
        "exact_staged_publish",
        staged_hash == published_hash,
        {"stagedSha256": staged_hash, "publishedSha256": published_hash},
    )
    record(
        "v82_only_deployment",
        not (ROOT / "PS1 game" / "Vigilante82PC.exe").exists(),
        {"ps1GameV82Executable": False},
    )

    final_summary = load_json(FINAL_RUN / "summary.json")
    final_runs = final_summary.get("runs", [])
    final_totals = final_summary.get("totals", {})
    expected_weapons = list(range(1, 8))
    expected_powerups = {
        "radar-jammer", "repair-wrench", "shield", "transform-1",
        "transform-2", "transform-3", "weapon-upgrade",
    }
    final_ok = (
        len(final_runs) == 1
        and bool(final_runs[0].get("passed"))
        and final_runs[0].get("actual_overlay") == "LEVELS_N64_DREAMLND"
        and final_runs[0].get("last_frame") == 2700
        and final_totals.get("weaponKindsArmed") == expected_weapons
        and final_totals.get("weaponKindsFired") == expected_weapons
        and set(final_totals.get("powerups", [])) == expected_powerups
        and len(final_totals.get("specialCommands", [])) == 18
        and final_totals.get("collisionStreamRejections") == 0
    )
    record(
        "dreamland_molo_full_schedule",
        final_ok,
        {
            "frames": final_runs[0].get("last_frame") if final_runs else None,
            "weaponsArmed": final_totals.get("weaponKindsArmed"),
            "weaponsFired": final_totals.get("weaponKindsFired"),
            "powerups": final_totals.get("powerups"),
            "specialCommands": len(final_totals.get("specialCommands", [])),
            "collisionStreamRejections": final_totals.get("collisionStreamRejections"),
        },
    )

    current_summary = load_json(CURRENT_DREAMLAND_RUN / "summary.json")
    current_runs = current_summary.get("runs", [])
    current_stdout = run_diagnostics_text(current_runs[0]) if current_runs else ""
    logged_hash = re.search(r"\bsha256=([0-9A-F]{64})\b", current_stdout)
    record(
        "current_dreamland_run_used_staged_binary",
        len(current_runs) == 1
        and bool(current_runs[0].get("passed"))
        and current_runs[0].get("actual_overlay") == "LEVELS_N64_DREAMLND"
        and current_runs[0].get("last_frame") == 900
        and logged_hash is not None
        and logged_hash.group(1) == staged_hash,
        {
            "frames": current_runs[0].get("last_frame") if current_runs else None,
            "loggedSha256": logged_hash.group(1) if logged_hash else None,
        },
    )

    conversion = load_json(CURRENT_DREAMLAND_DIAGNOSTICS)
    record(
        "dreamland_terrain_depth_fog_hud",
        bool(conversion.get("passed")) and all(conversion.get("checks", {}).values()),
        conversion.get("evidence"),
    )
    ui_runtime = load_json(FINAL_RUN / "native-ui-runtime-audit.json")
    ui_selector = ui_runtime.get("selector", {})
    ui_loading = ui_runtime.get("loading", {})
    record(
        "selector_loading_runtime_contract",
        ui_runtime.get("status") == "PASS"
        and ui_selector.get("resourceCount") == 20
        and ui_selector.get("previewCount") == 19
        and ui_selector.get("backgroundCount") == 1
        and ui_selector.get("selectedSlot") == 18
        and ui_loading.get("mdecFrames320x96") == 1
        and ui_loading.get("overlayLoads") == 1,
        {"selector": ui_selector, "loading": ui_loading},
    )
    ui_assets = load_json(ARTIFACTS / "native-arena-ui-audit-final-assets.json")
    asset_selector = ui_assets.get("selector", {})
    asset_loading = ui_assets.get("loading", {})
    record(
        "selector_loading_asset_contract",
        ui_assets.get("status") == "PASS"
        and asset_selector.get("crop") == "none"
        and asset_selector.get("dimensions") == [220, 74]
        and asset_selector.get("templateIndex") == 8
        and asset_selector.get("backgroundByteExact") is True
        and asset_loading.get("chunkCount") == 1
        and asset_loading.get("dimensions") == [320, 96]
        and ui_assets.get("presentationOverlayFiles") == [],
        {"selector": asset_selector, "loading": asset_loading},
    )

    final_material = load_json(CONVERTED_MATERIAL_RUN / "material-audit.json")
    record(
        "converted_vehicle_native_reflection",
        bool(final_material.get("passed"))
        and final_material.get("native_reflection_signatures", 0) > 0
        and final_material.get("forbidden_imported_material_signatures") == [],
        {
            "nativeReflectionSignatures": final_material.get("native_reflection_signatures"),
            "checks": final_material.get("checks"),
        },
    )
    stock_summary = load_json(STOCK_RUN / "summary.json")
    stock_material = load_json(STOCK_RUN / "vehicle-material-audit.json")
    stock_runs = stock_summary.get("runs", [])
    stock_stdout = run_diagnostics_text(stock_runs[0]) if stock_runs else ""
    stock_logged_hash = re.search(r"\bsha256=([0-9A-F]{64})\b", stock_stdout)
    record(
        "stock_v82_native_reflection_control",
        len(stock_runs) == 1
        and bool(stock_runs[0].get("passed"))
        and stock_logged_hash is not None
        and stock_logged_hash.group(1) == staged_hash
        and bool(stock_material.get("passed"))
        and stock_material.get("native_reflection_signatures", 0) > 0,
        {
            "frames": stock_runs[0].get("last_frame") if stock_runs else None,
            "nativeReflectionSignatures": stock_material.get("native_reflection_signatures"),
            "loggedSha256": stock_logged_hash.group(1) if stock_logged_hash else None,
        },
    )
    roster = load_json(ROSTER_RUN / "audit.json")
    revisited = roster.get("revisited_guests", {})
    roster_stdout = (ROSTER_RUN / "stdout.log").read_text(
        encoding="utf-8", errors="replace"
    )
    roster_logged_hash = re.search(
        r"\bsha256=([0-9A-F]{64})\b", roster_stdout
    )
    record(
        "all_13_og_v8_previews_rebuild_with_stable_native_materials",
        bool(roster.get("passed"))
        and roster.get("frame_count", 0) >= 7000
        and roster.get("created_preview_count", 0) >= 24
        and len(revisited) == 13
        and all(len(generations) >= 2 for generations in revisited.values())
        and all(roster.get("checks", {}).values())
        and roster_logged_hash is not None
        and roster_logged_hash.group(1) == staged_hash,
        {
            "vehicleCount": len(revisited),
            "renderFrames": roster.get("frame_count"),
            "previewConstructions": roster.get("created_preview_count"),
            "loggedSha256": (
                roster_logged_hash.group(1) if roster_logged_hash else None
            ),
            "checks": roster.get("checks"),
        },
    )

    runtime_sources = [
        ROOT / "tools/recompone-reference/RecompOne.Runtime/sdk/V82ArenaRegistry.cs",
        ROOT / "tools/recompone-reference/RecompOne.Runtime/sdk/V82Compat.cs",
        ROOT / "tools/recompone-reference/RecompOne.Runtime/sdk/V82VehicleRegistry.cs",
    ]
    runtime_sources.extend((
        ROOT / "tools/recompone-reference/RecompOne.Runtime/Gpu"
    ).rglob("*.cs"))
    forbidden_terms = (
        "V82DreamlandCompat", "ImportedGlass", "ImportedShadow",
        "ImportedVehiclePacketRanges", "IsImportedVehiclePacket",
        "n64.super_dreamland_64",
    )
    source_hits: dict[str, list[str]] = {}
    for path in runtime_sources:
        text = path.read_text(encoding="utf-8", errors="replace")
        hits = [term for term in forbidden_terms if term in text]
        if hits:
            source_hits[str(path.relative_to(ROOT))] = hits
    record(
        "no_map_or_car_specific_runtime_render_exceptions",
        not source_hits
        and not (
            ROOT / "tools/recompone-reference/RecompOne.Runtime/sdk/"
            "V82DreamlandCompat.cs"
        ).exists(),
        {"forbiddenSourceHits": source_hits, "compatibilityFileExists": False},
    )
    generated_main = (
        ROOT / "reference-v8-2/generated/recompiled/main.cs"
    ).read_text(encoding="utf-8", errors="replace")
    converted_overlay = (
        ROOT / "tools/recompone-v8-2/LEVELS_N64_DREAMLND.cs"
    ).read_text(encoding="utf-8", errors="replace")
    record(
        "static_overlay_has_no_runtime_compatibility_seam",
        all(term not in generated_main for term in (
            "V82DreamlandCompat", "TraceNativeModel", "TraceNativeObject"
        ))
        and "V82DreamlandCompat" not in converted_overlay
        and "0x8FFF01" not in converted_overlay,
        {"generatedMainCompatibilityMarkers": 0, "overlayHostServiceMarkers": 0},
    )

    arena_registry = (
        ROOT / "V8_2_LOOSE/mods/v82_n64_super_dreamland/ARENAS.V8R"
    )
    vehicle_registry = (
        ROOT / "V8_2_LOOSE/mods/v8_to_v82_guest_roster/VEHICLES.V8R"
    )
    record(
        "generic_registry_extensions_present",
        arena_registry.is_file() and vehicle_registry.is_file(),
        {
            "arenaRegistryBytes": arena_registry.stat().st_size,
            "vehicleRegistryBytes": vehicle_registry.stat().st_size,
        },
    )

    root_logs = [
        path.name for path in (ROOT / "V8_2_LOOSE").iterdir()
        if path.is_file()
        and (re.match(r"^v8(?:_|-).*\.log$", path.name, re.I) or path.name == "v8.log")
    ]
    reference_root = (ROOT / "artifacts/v8_to_v82_guest_roster").resolve()
    artifact_files, skipped_artifact_links = walk_files_without_reparse_points(
        ROOT / "artifacts"
    )
    generated_media = [
        str(path.relative_to(ROOT)) for path in artifact_files
        if path.is_file() and path.suffix.lower() in MEDIA
        and reference_root not in path.resolve().parents
    ]
    retained_references = [
        path for path in reference_root.rglob("*")
        if path.is_file() and path.suffix.lower() in MEDIA
    ]
    live_selector_assets = list((
        ROOT / "V8_2_LOOSE/mods/v8_to_v82_guest_roster/SHELL"
    ).glob("SELECTOR_*.PPM"))
    record(
        "logs_and_generated_media_clean",
        not root_logs and not generated_media
        and len(retained_references) == 13 and len(live_selector_assets) == 13,
        {
            "rootLogs": root_logs,
            "generatedMediaOutsideReferences": generated_media,
            "skippedArtifactReparsePoints": [
                str(path.relative_to(ROOT)) for path in skipped_artifact_links
            ],
            "requiredConverterReferences": len(retained_references),
            "requiredLiveSelectorAssets": len(live_selector_assets),
        },
    )

    commands = {
        "unitTests": run([
            sys.executable, "-m", "unittest",
            "tools/blender_addons/tests/test_v8_to_v82_conversion.py",
            "tools/recompone-v8-2/test_port_dreamland_overlay.py",
        ]),
        "assetAudit": run([
            sys.executable,
            "tools/recompone-v8-2/audit_native_arena_ui.py",
            "--output",
            str(ARTIFACTS / "native-arena-ui-audit-final-assets.json"),
        ]),
        "vehicleContractAudit": run([
            sys.executable,
            "tools/recompone-v8-2/audit_vehicle_conversion_contracts.py",
        ]),
        "diffCheck": run(["git", "diff", "--check"]),
    }
    record(
        "current_converter_and_source_tests_pass",
        all(result["exitCode"] == 0 for result in commands.values()),
        commands,
    )

    passed = all(checks.values())
    report = {
        "schema": 1,
        "technicalStatus": "PASS" if passed else "FAIL",
        "goalState": "READY_FOR_EXPLICIT_REGULAR_REVIEW" if passed else "ACTIVE",
        "goalComplete": False,
        "checks": checks,
        "evidence": evidence,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        f"[NativeConversionReviewGate] {report['technicalStatus']} "
        f"checks={sum(checks.values())}/{len(checks)} "
        f"goal={report['goalState']} complete=0"
    )
    for name, value in checks.items():
        print(f"  {'PASS' if value else 'FAIL'} {name}")
    print(f"  report={args.output.resolve()}")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
