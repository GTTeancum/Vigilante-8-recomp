#!/usr/bin/env python3
"""Verify the current staged Y/Florida goal without retaining new images."""

from __future__ import annotations

import argparse
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
CAROUSEL = ARTIFACTS / "v82-y-final-full-carousel-silent-v2"
REENTRY = ARTIFACTS / "v82-y-florida-dreamland-reentry-final-silent"
FLORIDA_HOVER = ARTIFACTS / "v82-y-florida-water-hover-final-silent"
DREAMLAND_HOVER = ARTIFACTS / "v82-y-dreamland-water-hover-final-silent"
FLORIDA_FIDELITY = (
    ARTIFACTS / "v82-florida-fidelity-camera-depth-gl-silent"
)
FLORIDA_CLOSE = ARTIFACTS / "v82-florida-close-scenery-gl-silent"
FLORIDA_OBJECT_CULL = ARTIFACTS / "v82-florida-object-cull-gl-silent"
SHARED_FOG = (
    ARTIFACTS / "v82-shared-fog-regression-bayou-harbor-gl-silent-v2"
)
VISUAL_EVIDENCE = ARTIFACTS / "florida-fidelity-review-20260824"
SOURCE_REFERENCE = "https://www.youtube.com/watch?v=PlaHZTzbBnA"
FATAL = re.compile(
    r"\[Fatal\]|Unhandled exception|unmapped call|out of vram",
    re.IGNORECASE,
)


def load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def files_with_suffix(root: Path, suffix: str) -> list[Path]:
    return sorted(path for path in root.glob(f"*{suffix}") if path.is_file())


def text(paths: list[Path]) -> str:
    return "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in paths
    )


def logged_hash(stdout_path: Path) -> str | None:
    first = stdout_path.read_text(
        encoding="utf-8", errors="replace"
    ).splitlines()[:3]
    match = re.search(r"sha256=([0-9A-Fa-f]{64})", "\n".join(first))
    return match.group(1).upper() if match else None


def soak_evidence(root: Path, staged_hash: str) -> dict[str, object]:
    summary = load_json(root / "summary.json")
    runs = summary.get("runs", [])
    totals = summary.get("totals", {})
    stdout_paths = files_with_suffix(root, ".stdout.log")
    stderr_paths = files_with_suffix(root, ".stderr.log")
    combined = text(stdout_paths + stderr_paths)
    hashes = [logged_hash(path) for path in stdout_paths]
    return {
        "passed": (
            bool(runs)
            and all(bool(run.get("passed")) for run in runs)
            and totals.get("failed") == 0
            and all(value == staged_hash for value in hashes)
            and FATAL.search(combined) is None
        ),
        "runs": len(runs),
        "frames": [run.get("last_frame") for run in runs],
        "overlays": [run.get("actual_overlay") for run in runs],
        "loggedSha256": hashes,
        "collisionStreamRejections": totals.get(
            "collisionStreamRejections"
        ),
        "stdout": stdout_paths,
        "stderr": stderr_paths,
        "text": combined,
    }


def hover_evidence(root: Path, staged_hash: str) -> dict[str, object]:
    base = soak_evidence(root, staged_hash)
    combined = str(base.pop("text"))
    controller_rows = re.findall(
        r"\[V82VehicleController\].*id=guest\.v8\.y_the_alien.*"
        r"controller=flying.*water-plane=(-?\d+).*water-active=1.*"
        r"surface-y=(-?\d+)",
        combined,
    )
    water_surface_rows = sum(
        surface == plane for plane, surface in controller_rows
    )
    lifecycle = (
        "[V82WaterLifecycle] timeout frame=270 elapsed=240 "
        "destroyed=0 respawned=0"
    ) in combined
    passed = bool(base["passed"] and len(controller_rows) >= 6 and
                  water_surface_rows >= 4 and lifecycle)
    base.update({
        "passed": passed,
        "flyingWaterRows": len(controller_rows),
        "nativeWaterSurfaceRows": water_surface_rows,
        "destroyed": False if lifecycle else None,
        "respawned": False if lifecycle else None,
    })
    for key in ("stdout", "stderr"):
        base[key] = [str(path) for path in base[key]]
    return base


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=ARTIFACTS / "v82-y-florida-review-gate.json",
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
        {
            "stagedSha256": staged_hash,
            "publishedSha256": published_hash,
            "stagedBytes": STAGED_EXE.stat().st_size,
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
        "conversion_package_contract",
        validator.returncode == 0
        and "PASS: 13 vehicles, 39 exclusive native banks" in validator_text
        and "byte-exact Blender round trip" in validator_text,
        {
            "exitCode": validator.returncode,
            "output": validator_text.strip(),
        },
    )

    carousel = load_json(CAROUSEL / "acceptance.json")
    selector_log = CAROUSEL / "stderr.log"
    y_rows = 0
    y_support_rows = 0
    y_min = None
    y_max = None
    with selector_log.open(encoding="utf-8", errors="replace") as stream:
        for line in stream:
            if "[V82SelectorPhysics] guest=12 " not in line:
                continue
            y_rows += 1
            if all(
                f"w{index}=0x00000000" not in line
                and f"w{index}=0x" in line
                for index in range(4)
            ):
                y_support_rows += 1
            match = re.search(r"pos=\([^,]+,(-?\d+),", line)
            if match:
                value = int(match.group(1))
                y_min = value if y_min is None else min(y_min, value)
                y_max = value if y_max is None else max(y_max, value)
    selector_pass = (
        bool(carousel.get("passed"))
        and carousel.get("executable_sha256") == staged_hash
        and bool(carousel.get("complete_double_roster"))
        and bool(carousel.get("carousel_order_passed"))
        and bool(carousel.get("enemy_stage"))
        and bool(carousel.get("clean_exit"))
        and y_rows >= 180
        and y_rows == y_support_rows
        and y_min is not None
        and y_min > 3_000_000
        and y_max is not None
        and y_max < 3_250_000
    )
    record(
        "selector_collision_settling_and_complete_carousel",
        selector_pass,
        {
            "carouselTransitions": len(
                carousel.get("carousel_transitions", [])
            ),
            "completeDoubleRoster": carousel.get(
                "complete_double_roster"
            ),
            "yPhysicsRows": y_rows,
            "yFourSupportRows": y_support_rows,
            "yPositionRange": [y_min, y_max],
            "enemyStage": carousel.get("enemy_stage"),
            "cleanExit": carousel.get("clean_exit"),
        },
    )

    florida_hover = hover_evidence(FLORIDA_HOVER, staged_hash)
    dreamland_hover = hover_evidence(DREAMLAND_HOVER, staged_hash)
    record("y_hover_over_florida_water", bool(florida_hover["passed"]),
           florida_hover)
    record("y_hover_over_dreamland_water", bool(dreamland_hover["passed"]),
           dreamland_hover)

    reentry_stdout = (REENTRY / "stdout.log").read_text(
        encoding="utf-8", errors="replace"
    )
    reentry_stderr = (REENTRY / "stderr.log").read_text(
        encoding="utf-8", errors="replace"
    )
    reentry_text = reentry_stdout + "\n" + reentry_stderr
    reentry_markers = {
        "current_hash": f"sha256={staged_hash}" in reentry_text,
        "first_location": "[V82Arena] location=retail.3" in reentry_text,
        "relocated_shell":
            "loaded relocated overlay: SHELL_SHELL" in reentry_text,
        "second_generation":
            "native locations generation=2" in reentry_text
            and "baseline-slot=3" in reentry_text,
        "dreamland_selected":
            "[V82Arena] location=n64.super_dreamland_64" in reentry_text,
        "dreamland_launched":
            "path=Levels\\N64\\DreamLnd.exp" in reentry_text,
        "y_selector_twice": reentry_text.count(
            "created guest.v8.y_the_alien native-selector"
        ) >= 2,
        "y_gameplay_twice": reentry_text.count(
            "created guest.v8.y_the_alien identity=76"
        ) >= 2,
        "replay_completed":
            "deterministic replay completed at poll 13800" in reentry_text,
        "no_fatal": FATAL.search(reentry_text) is None,
    }
    record(
        "florida_exit_dreamland_y_reentry",
        all(reentry_markers.values()),
        reentry_markers,
    )

    florida = soak_evidence(FLORIDA_FIDELITY, staged_hash)
    florida_text = str(florida.pop("text"))
    fog_windows = len(re.findall(
        r"\[EnhancedFogFrame\].*valid=1", florida_text
    ))
    renderer_windows = len(re.findall(
        r"\[EnhancedRenderer\].*triangles=[1-9]\d*", florida_text
    ))
    shader = (
        ROOT / "tools" / "recompone-reference" / "RecompOne.Runtime" /
        "Gpu" / "Enhanced" / "GlShaders.cs"
    ).read_text(encoding="utf-8")
    shader_contract = (
        "float fogDepth = max(vDepth, 1.0);" in shader
        and "float fogDepth = rasterCameraDepth();" not in shader
    )
    visual_files = {
        path.name: {
            "sha256": digest(path),
            "bytes": path.stat().st_size,
        }
        for path in sorted(VISUAL_EVIDENCE.glob("*.png"))
    }
    florida_pass = bool(
        florida["passed"]
        and florida["frames"] == [360]
        and "hle=True" in florida_text
        and fog_windows >= 10
        and renderer_windows >= 10
        and shader_contract
        and len(visual_files) == 3
    )
    florida.update({
        "passed": florida_pass,
        "enhancedGl": "hle=True" in florida_text,
        "validFogWindows": fog_windows,
        "activeRendererWindows": renderer_windows,
        "cameraDepthShaderContract": shader_contract,
        "sourceReference": SOURCE_REFERENCE,
        "sourceTimestampsSeconds": [0, 116],
        "visualEvidence": visual_files,
        "sequentialVisualAssessment": {
            "palette": "warm brown/olive terrain and readable structures",
            "sky": "pale cyan/yellow authored backdrop retained",
            "fog": "distance haze retained without scene-wide grey wash",
        },
    })
    for key in ("stdout", "stderr"):
        florida[key] = [str(path) for path in florida[key]]
    record("florida_source_referenced_fog_fidelity", florida_pass, florida)

    close = soak_evidence(FLORIDA_CLOSE, staged_hash)
    close_text = str(close.pop("text"))
    cull = soak_evidence(FLORIDA_OBJECT_CULL, staged_hash)
    cull_text = str(cull.pop("text"))
    object_pops = cull_text.count("[V82ObjectPop]")
    gate_rows = re.findall(
        r"\[V82ObjectGates\].*emptyFrustum=(\d+)", cull_text
    )
    frustum_empty = sum(int(value) for value in gate_rows)
    close_pass = bool(
        close["passed"]
        and close["frames"] == [720]
        and cull["passed"]
        and object_pops == 0
        and bool(gate_rows)
        and frustum_empty == 0
        and FATAL.search(close_text) is None
    )
    record(
        "florida_close_scenery_clipping_pass",
        close_pass,
        {
            "closeSweepFrames": close["frames"],
            "objectCullFrames": cull["frames"],
            "objectPopEvents": object_pops,
            "objectGateReports": len(gate_rows),
            "frustumEmptyObjects": frustum_empty,
            "flaggedFrameAssessment": (
                "telemetry flags coincide with geometry entering/leaving the "
                "widened camera edge; no interior object disappearance was "
                "present in the exact seven-frame sequence"
            ),
        },
    )

    shared = soak_evidence(SHARED_FOG, staged_hash)
    shared_text = str(shared.pop("text"))
    shared_pass = bool(
        shared["passed"]
        and shared["frames"] == [360, 360]
        and shared["overlays"] == ["LEVELS_BAYOU", "LEVELS_HARBOR"]
        and "hle=True" in shared_text
    )
    for key in ("stdout", "stderr"):
        shared[key] = [str(path) for path in shared[key]]
    shared.update({"passed": shared_pass, "enhancedGl": "hle=True" in shared_text})
    record("shared_fog_regression_controls", shared_pass, shared)

    forbidden = re.compile(
        r"\b(?:if|switch)\s*\([^\r\n)]*"
        r"(?:y_the_alien|florida|dreamland|levels[_\\/]+launch)|"
        r"\bcase\s+[^:\r\n]*"
        r"(?:y_the_alien|florida|dreamland)",
        re.IGNORECASE,
    )
    runtime_sources = [
        *(
            ROOT / "tools" / "recompone-reference" /
            "RecompOne.Runtime" / "sdk"
        ).glob("V82*.cs"),
        *(
            ROOT / "tools" / "recompone-reference" /
            "RecompOne.Runtime" / "Gpu"
        ).rglob("*.cs"),
    ]
    forbidden_hits: dict[str, list[str]] = {}
    for path in runtime_sources:
        hits = [
            f"{index}:{line.strip()}"
            for index, line in enumerate(
                path.read_text(encoding="utf-8").splitlines(), 1
            )
            if forbidden.search(line)
        ]
        if hits:
            forbidden_hits[str(path.relative_to(ROOT))] = hits
    capability_source = (
        ROOT / "tools" / "recompone-reference" / "RecompOne.Runtime" /
        "sdk" / "V82VehicleRegistry.cs"
    ).read_text(encoding="utf-8")
    conversion_source = (
        ROOT / "tools" / "blender_addons" /
        "vigilante8_vehicle_tools" / "conversion.py"
    ).read_text(encoding="utf-8")
    converter_contacts = "def add_v82_contact_anchors(" in conversion_source
    generic_capabilities = all(value in capability_source for value in (
        "UsesFlyingController",
        "SupportsTransformations",
    )) and converter_contacts
    record(
        "no_content_specific_runtime_exceptions",
        not forbidden_hits and generic_capabilities,
        {
            "forbiddenConditionalHits": forbidden_hits,
            "genericFlyingCapability": "UsesFlyingController" in capability_source,
            "genericTransformationCapability":
                "SupportsTransformations" in capability_source,
            "conversionOwnsSelectorContacts": converter_contacts,
        },
    )

    root_logs = sorted(
        path.name for path in (ROOT / "V8_2_LOOSE").glob("v8*.log")
    )
    record(
        "deployment_boundary",
        not (ROOT / "PS1 game" / "Vigilante82PC.exe").exists(),
        {
            "v82ExecutableInPs1Root":
                (ROOT / "PS1 game" / "Vigilante82PC.exe").exists(),
            "stagedRootLogsBeforeFinalCleanup": root_logs,
        },
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
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        f"[YFloridaReviewGate] {report['technicalStatus']} "
        f"checks={sum(checks.values())}/{len(checks)} "
        "review=PENDING_USER_REVIEW goal=ACTIVE complete=0"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    print(f"  report={output}")
    return 0 if technical else 1


if __name__ == "__main__":
    raise SystemExit(main())
