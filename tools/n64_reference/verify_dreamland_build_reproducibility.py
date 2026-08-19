#!/usr/bin/env python3
"""Verify the complete V8 CUE-to-RecompOne build provenance chain."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


SOURCE_SUFFIXES = {
    ".cs",
    ".csproj",
    ".json",
    ".props",
    ".py",
    ".svg",
    ".targets",
}
EXCLUDED_PARTS = {"bin", "obj", "__pycache__"}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def iso_mtime(path: Path) -> str:
    return datetime.fromtimestamp(
        path.stat().st_mtime, timezone.utc
    ).isoformat()


def tree_digest(root: Path, *, generated: bool = False) -> dict[str, object]:
    if generated:
        files = sorted(
            path
            for path in root.iterdir()
            if path.is_file() and path.suffix.lower() in {".cs", ".csproj"}
        )
    else:
        files = sorted(
            path
            for path in root.rglob("*")
            if path.is_file()
            and path.suffix.lower() in SOURCE_SUFFIXES
            and not EXCLUDED_PARTS.intersection(path.relative_to(root).parts)
        )
    digest = hashlib.sha256()
    for path in files:
        relative = path.relative_to(root).as_posix().encode("utf-8")
        digest.update(len(relative).to_bytes(4, "little"))
        digest.update(relative)
        digest.update(bytes.fromhex(sha256(path)))
    return {
        "root": str(root.resolve()),
        "files": len(files),
        "sha256": digest.hexdigest().upper(),
    }


def run_check(script: Path, root: Path) -> dict[str, object]:
    process = subprocess.run(
        [sys.executable, str(script), "--check"],
        cwd=root,
        capture_output=True,
        text=True,
        check=False,
    )
    return {
        "script": str(script.resolve()),
        "exit_code": process.returncode,
        "stdout": process.stdout.strip(),
        "stderr": process.stderr.strip(),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cue", type=Path, required=True)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--generated-dir", type=Path, required=True)
    parser.add_argument("--first-exe", type=Path, required=True)
    parser.add_argument("--second-exe", type=Path, required=True)
    parser.add_argument("--deployed-exe", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[2]
    cue = args.cue.resolve()
    config_path = args.config.resolve()
    generated_dir = args.generated_dir.resolve()
    config = json.loads(config_path.read_text(encoding="utf-8-sig"))
    configured_cue = (config_path.parent / config["cue"]).resolve()

    cue_text = cue.read_text(encoding="utf-8-sig")
    track_names = re.findall(r'^FILE\s+"([^"]+)"', cue_text, re.MULTILINE)
    tracks = []
    missing_tracks = []
    for name in track_names:
        path = cue.parent / name
        if not path.is_file():
            missing_tracks.append(str(path))
            continue
        tracks.append(
            {
                "path": str(path.resolve()),
                "bytes": path.stat().st_size,
                "sha256": sha256(path),
            }
        )

    generated_files = sorted(
        path
        for path in generated_dir.iterdir()
        if path.is_file() and path.suffix.lower() in {".cs", ".csproj"}
    )
    generated_min_mtime = min(path.stat().st_mtime_ns for path in generated_files)
    generated_max_mtime = max(path.stat().st_mtime_ns for path in generated_files)
    first_mtime = args.first_exe.stat().st_mtime_ns
    second_mtime = args.second_exe.stat().st_mtime_ns

    prepare_script = root / "tools/recompone-v8/prepare_reference.py"
    vehicle_patch = root / "tools/recompone-v8/apply_vehicle_engine_patches.py"
    location_patch = root / "tools/recompone-v8/apply_native_location_patches.py"
    patch_checks = [
        run_check(vehicle_patch, root),
        run_check(location_patch, root),
    ]

    executable_paths = {
        "first_end_to_end": args.first_exe.resolve(),
        "second_end_to_end": args.second_exe.resolve(),
        "deployed": args.deployed_exe.resolve(),
    }
    executables = {
        name: {
            "path": str(path),
            "bytes": path.stat().st_size,
            "sha256": sha256(path),
            "mtime_utc": iso_mtime(path),
        }
        for name, path in executable_paths.items()
    }
    executable_hashes = {entry["sha256"] for entry in executables.values()}
    executable_sizes = {entry["bytes"] for entry in executables.values()}

    main_source = (generated_dir / "main.cs").read_text(encoding="utf-8")
    required_generated_seams = {
        "deterministic_rng_seed_prehook": (
            "V8Compat.ApplyDeterministicGameRngSeed" in main_source
        ),
        "guest_vram_allocation_prehook": (
            "V8Compat.ClaimGuestVramAllocation" in main_source
        ),
    }

    failures = []
    if configured_cue != cue:
        failures.append("generated config does not reference the audited CUE")
    if len(track_names) != 13 or len(tracks) != 13 or missing_tracks:
        failures.append("CUE track closure is incomplete")
    if len(executable_hashes) != 1 or len(executable_sizes) != 1:
        failures.append("end-to-end builds and deployment are not byte-identical")
    if not first_mtime < generated_min_mtime:
        failures.append("first build does not predate the second generated-source set")
    if not generated_max_mtime < second_mtime:
        failures.append("second build does not follow the second generated-source set")
    if any(check["exit_code"] != 0 for check in patch_checks):
        failures.append("generated post-patch checks failed")
    if not all(required_generated_seams.values()):
        failures.append("required generated prehooks are absent")

    report = {
        "schema": "v8.dreamland-build-reproducibility.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "pipeline": [
            "prepare_reference.py --cue <cue>",
            "RecompOne.Recompiler -c Release -- <generated-config>",
            "apply_vehicle_engine_patches.py",
            "apply_native_location_patches.py",
            "clean generated Release bin/obj",
            "dotnet publish generated Vigilante8PC.csproj -c Release",
        ],
        "cue": {
            "path": str(cue),
            "sha256": sha256(cue),
            "configured_path": str(configured_cue),
            "tracks": tracks,
            "missing_tracks": missing_tracks,
        },
        "generator_inputs": {
            "prepare_reference_sha256": sha256(prepare_script),
            "vehicle_patch_sha256": sha256(vehicle_patch),
            "location_patch_sha256": sha256(location_patch),
            "recompiler": tree_digest(
                root / "tools/recompone-reference/RecompOne.Recompiler"
            ),
            "runtime": tree_digest(
                root / "tools/recompone-reference/RecompOne.Runtime"
            ),
            "generated_config_sha256": sha256(config_path),
        },
        "generated_sources": {
            **tree_digest(generated_dir, generated=True),
            "earliest_mtime_utc": datetime.fromtimestamp(
                generated_min_mtime / 1_000_000_000, timezone.utc
            ).isoformat(),
            "latest_mtime_utc": datetime.fromtimestamp(
                generated_max_mtime / 1_000_000_000, timezone.utc
            ).isoformat(),
            "required_seams": required_generated_seams,
            "post_patch_checks": patch_checks,
        },
        "independent_generation_order": {
            "first_build_predates_second_generated_sources": (
                first_mtime < generated_min_mtime
            ),
            "second_generated_sources_predate_second_build": (
                generated_max_mtime < second_mtime
            ),
        },
        "executables": executables,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
