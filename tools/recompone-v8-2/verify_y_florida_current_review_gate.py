#!/usr/bin/env python3
"""Verify the exact staged Y/Florida review candidate without new captures."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
ARTIFACTS = ROOT / "artifacts"
STAGED_EXE = ROOT / "V8_2_LOOSE" / "Vigilante82PC.exe"
PUBLISHED_EXE = (
    ROOT / "reference-v8-2" / "generated" / "recompiled" / "bin" /
    "Release" / "net10.0" / "win-x64" / "publish" /
    "Vigilante82PC.exe"
)
SELECTOR = ARTIFACTS / "v82-y-selector-inert-contact-final-silent"
CAROUSEL = ARTIFACTS / "v82-y-full-carousel-inert-contact-final-silent"
FLORIDA_HOVER = ARTIFACTS / "v82-y-florida-water-hover-current-silent"
DREAMLAND_HOVER = ARTIFACTS / "v82-y-dreamland-water-hover-current-silent"
REENTRY = ARTIFACTS / "v82-y-reentry-inert-contact-final-silent"
LOADING = ARTIFACTS / "v82-loading-card-florida-final"
FLORIDA = ARTIFACTS / "v82-florida-bright-fog-final"
SHARED = ARTIFACTS / "v82-shared-fog-controls-final-360"
PROOFS = ARTIFACTS / "v82-current-review-proofs"
OUTPUT = ARTIFACTS / "v82-y-florida-current-review-gate.json"
FATAL = re.compile(
    r"\[Fatal\]|Unhandled exception|unmapped call|out of vram",
    re.IGNORECASE,
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def read_logs(root: Path) -> str:
    paths = sorted(root.glob("*.log"))
    return "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in paths
    )


def soak_pass(root: Path, staged_hash: str, overlays: list[str]) -> tuple[bool, dict[str, object], str]:
    summary = load_json(root / "summary.json")
    runs = summary.get("runs", [])
    text = read_logs(root)
    headers = re.findall(r"sha256=([0-9A-Fa-f]{64})", text)
    passed = bool(
        len(runs) == len(overlays)
        and all(bool(run.get("passed")) for run in runs)
        and [run.get("actual_overlay") for run in runs] == overlays
        and all(value.upper() == staged_hash for value in headers)
        and FATAL.search(text) is None
    )
    return passed, summary, text


def hover_pass(root: Path, staged_hash: str, overlay: str) -> tuple[bool, dict[str, object]]:
    passed, summary, text = soak_pass(root, staged_hash, [overlay])
    controller_rows = re.findall(
        r"\[V82VehicleController\].*id=guest\.v8\.y_the_alien.*"
        r"controller=flying.*water-plane=(-?\d+).*water-active=1.*"
        r"surface-y=(-?\d+)",
        text,
    )
    surface_rows = sum(plane == surface for plane, surface in controller_rows)
    lifecycle = (
        "[V82WaterLifecycle] timeout frame=270 elapsed=240 "
        "destroyed=0 respawned=0"
    ) in text
    return bool(passed and len(controller_rows) >= 6 and surface_rows >= 4 and lifecycle), {
        "frames": [run.get("last_frame") for run in summary.get("runs", [])],
        "flyingWaterRows": len(controller_rows),
        "nativeWaterSurfaceRows": surface_rows,
        "destroyed": False if lifecycle else None,
        "respawned": False if lifecycle else None,
    }


def main() -> int:
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
        {
            "stagedSha256": staged_hash,
            "publishedSha256": published_hash,
            "bytes": STAGED_EXE.stat().st_size,
        },
    )

    validator = subprocess.run(
        [
            sys.executable,
            str(
                ROOT / "tools" / "blender_addons" / "tests" /
                "validate_v8_to_v82_guest_roster.py"
            ),
        ],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )
    validator_text = validator.stdout + validator.stderr
    record(
        "conversion_package_and_inert_contact_bank",
        validator.returncode == 0
        and "PASS: 13 vehicles, 39 exclusive native banks" in validator_text
        and "byte-exact Blender round trip" in validator_text,
        {"exitCode": validator.returncode, "output": validator_text.strip()},
    )

    selector = load_json(SELECTOR / "acceptance.json")
    record(
        "y_selector_support_without_rendered_wheels",
        bool(
            selector.get("passed")
            and selector.get("executable_sha256") == staged_hash
            and selector.get("physics_rows") == 220
            and selector.get("initial_physics", {}).get("supports") == 4
            and selector.get("final_physics", {}).get("supports") == 4
            and selector.get("clean_exit")
            and selector.get("retained_images") == 0
        ),
        {
            "physicsRows": selector.get("physics_rows"),
            "initial": selector.get("initial_physics"),
            "final": selector.get("final_physics"),
            "conversionContract": "one inert zero-packet group; collision stream retained",
        },
    )

    carousel = load_json(CAROUSEL / "acceptance.json")
    record(
        "complete_character_carousel",
        bool(
            carousel.get("passed")
            and carousel.get("executable_sha256") == staged_hash
            and carousel.get("complete_double_roster")
            and carousel.get("carousel_order_passed")
            and len(carousel.get("carousel_transitions", [])) == 44
            and carousel.get("preview_build_counts", {}).get(
                "guest.v8.y_the_alien") == 2
            and carousel.get("enemy_stage")
            and carousel.get("clean_exit")
            and carousel.get("vehicle_materials_passed")
        ),
        {
            "transitions": len(carousel.get("carousel_transitions", [])),
            "yBuilds": carousel.get("preview_build_counts", {}).get(
                "guest.v8.y_the_alien"),
            "enemyStage": carousel.get("enemy_stage"),
            "cleanExit": carousel.get("clean_exit"),
        },
    )

    florida_hover_pass, florida_hover = hover_pass(
        FLORIDA_HOVER, staged_hash, "LEVELS_LAUNCH")
    dreamland_hover_pass, dreamland_hover = hover_pass(
        DREAMLAND_HOVER, staged_hash, "LEVELS_N64_DREAMLND")
    record("y_hover_over_florida_water", florida_hover_pass, florida_hover)
    record("y_hover_over_dreamland_water", dreamland_hover_pass, dreamland_hover)

    reentry_text = read_logs(REENTRY)
    reentry_markers = {
        "currentHash": f"sha256={staged_hash}" in reentry_text,
        "florida": "[V82Arena] location=retail.3" in reentry_text,
        "returnedShell": "loaded relocated overlay: SHELL_SHELL" in reentry_text,
        "generationTwo": "native locations generation=2" in reentry_text,
        "dreamland": "[V82Arena] location=n64.super_dreamland_64" in reentry_text,
        "twoYSelectors": reentry_text.count(
            "created guest.v8.y_the_alien native-selector") >= 2,
        "twoYGameplayObjects": reentry_text.count(
            "created guest.v8.y_the_alien identity=76") >= 2,
        "completed": "deterministic replay completed at poll 13800" in reentry_text,
        "noFatal": FATAL.search(reentry_text) is None,
    }
    record("florida_exit_dreamland_y_reentry", all(reentry_markers.values()), reentry_markers)

    loading = load_json(LOADING / "acceptance.json")
    loading_card = loading.get("cards", [{}])[0]
    record(
        "generic_high_resolution_loading_card",
        bool(
            loading.get("passed")
            and loading.get("executableSha256") == staged_hash
            and loading.get("completeCardSetTotal") == 18
            and loading_card.get("arena") == "LEVELS_LAUNCH"
            and float(loading_card.get("correlation", 0.0)) >= 0.9998
        ),
        {
            "arena": loading_card.get("arena"),
            "correlation": loading_card.get("correlation"),
            "completeCardSet": loading.get("completeCardSetTotal"),
        },
    )

    florida_pass, florida_summary, florida_text = soak_pass(
        FLORIDA, staged_hash, ["LEVELS_LAUNCH"])
    fog_rows = re.findall(
        r"\[EnhancedFogFrame\].*valid=1.*near-white=(\d).*"
        r"rgb=([0-9.]+),([0-9.]+),([0-9.]+)",
        florida_text,
    )
    texture_pack = (
        "loaded 4335 loose DDS files / 34345 regions / 138 route regions / "
        "19 terrain atlases" in florida_text
        and "selected loading card overlay arena=LEVELS_LAUNCH" in florida_text
        and "active terrain atlas=levels/launch" in florida_text
        and "terrain atlas hit" in florida_text
        and re.search(r"\[TexturePack\] runtime hits=[1-9]\d*/", florida_text)
        is not None
    )
    authored_fog = bool(
        len(fog_rows) >= 10
        and all(row[0] == "0" for row in fog_rows)
        and all(
            abs(float(row[1]) - 0.819608) < 0.00001
            and abs(float(row[2]) - 0.945098) < 0.00001
            and abs(float(row[3]) - 0.901961) < 0.00001
            for row in fog_rows
        )
    )
    florida_frames = [
        run.get("last_frame") for run in florida_summary.get("runs", [])
    ]
    record(
        "florida_texture_pack_and_nonwhite_distance_fog",
        bool(florida_pass and florida_frames == [360] and texture_pack and authored_fog),
        {
            "frames": florida_frames,
            "texturePackLoaded": texture_pack,
            "validFogRows": len(fog_rows),
            "selectedRgb": [209, 241, 230],
            "nearWhiteSelected": any(row[0] == "1" for row in fog_rows),
        },
    )

    shared_pass, shared_summary, shared_text = soak_pass(
        SHARED, staged_hash, ["LEVELS_BAYOU", "LEVELS_HARBOR"])
    shared_frames = [
        run.get("last_frame") for run in shared_summary.get("runs", [])
    ]
    record(
        "shared_fog_controls",
        bool(
            shared_pass
            and shared_frames == [360, 360]
            and shared_text.count("[EnhancedFogFrame]") >= 20
        ),
        {"overlays": ["LEVELS_BAYOU", "LEVELS_HARBOR"], "frames": shared_frames},
    )

    forbidden = re.compile(
        r"\b(?:if|switch)\s*\([^\r\n)]*"
        r"(?:y_the_alien|florida|dreamland|levels[_\\/]+launch)|"
        r"\bcase\s+[^:\r\n]*"
        r"(?:y_the_alien|florida|dreamland)",
        re.IGNORECASE,
    )
    runtime_sources = [
        *(ROOT / "tools" / "recompone-reference" / "RecompOne.Runtime" / "sdk").glob("V82*.cs"),
        *(ROOT / "tools" / "recompone-reference" / "RecompOne.Runtime" / "Gpu").rglob("*.cs"),
        ROOT / "tools" / "recompone-reference" / "RecompOne.Runtime" /
            "Host" / "Window" / "PresentationRenderer.cs",
    ]
    forbidden_hits: dict[str, list[str]] = {}
    for path in runtime_sources:
        hits = [
            f"{line_number}:{line.strip()}"
            for line_number, line in enumerate(
                path.read_text(encoding="utf-8").splitlines(), 1)
            if forbidden.search(line)
        ]
        if hits:
            forbidden_hits[str(path.relative_to(ROOT))] = hits
    record(
        "no_content_specific_runtime_exceptions",
        not forbidden_hits,
        {"forbiddenConditionalHits": forbidden_hits},
    )

    proof_files = {
        path.name: {"sha256": digest(path), "bytes": path.stat().st_size}
        for path in sorted(PROOFS.glob("*.png"))
    }
    record(
        "compact_review_proofs",
        set(proof_files) == {
            "y_selector_no_wheels.png",
            "florida_loading_card.png",
            "florida_distance_fog.png",
        },
        proof_files,
    )

    root_logs = sorted(
        path.name for path in (ROOT / "V8_2_LOOSE").glob("v8*.log")
    )
    ps1_exe = ROOT / "PS1 game" / "Vigilante82PC.exe"
    record(
        "deployment_boundary",
        not root_logs and not ps1_exe.exists(),
        {"v82RootLogs": root_logs, "v82ExecutableInPs1Root": ps1_exe.exists()},
    )

    technical = all(checks.values())
    report = {
        "schema": 1,
        "technicalStatus": "PASS" if technical else "FAIL",
        "reviewStatus": "PENDING_USER_REVIEW",
        "goalState": "ACTIVE",
        "goalComplete": False,
        "checks": checks,
        "evidence": evidence,
    }
    OUTPUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        f"[YFloridaCurrentReviewGate] {report['technicalStatus']} "
        f"checks={sum(checks.values())}/{len(checks)} "
        "review=PENDING_USER_REVIEW goal=ACTIVE complete=0"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"  report={OUTPUT}")
    return 0 if technical else 1


if __name__ == "__main__":
    raise SystemExit(main())
