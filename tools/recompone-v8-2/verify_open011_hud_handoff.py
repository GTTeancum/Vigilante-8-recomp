#!/usr/bin/env python3
"""Verify the V82-OPEN-011 imported-NPC target HUD handoff evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from typing import Any


EXPECTED_SHA256 = (
    "EBE9693D4E24BF50342047C3E5D9E4B25951614650169CF0A8DE159FF4064F08"
)
EXPECTED_GUESTS = list(range(12))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def png_size(path: Path) -> list[int]:
    with path.open("rb") as stream:
        header = stream.read(24)
    if len(header) < 24 or header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path} is not a PNG")
    width, height = struct.unpack(">II", header[16:24])
    return [width, height]


def find_status_block(source: str) -> str:
    start = source.index("bool statusHudBacking =")
    end = source.index("if (statusHudBacking)", start)
    return source[start:end]


def has_all(text: str, needles: list[str]) -> tuple[bool, list[str]]:
    missing = [needle for needle in needles if needle not in text]
    return not missing, missing


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
    )
    parser.add_argument(
        "--current-acceptance",
        type=Path,
        default=None,
    )
    args = parser.parse_args()

    root = args.root.resolve()
    output = args.output or (
        root
        / "artifacts/v82-open011-hud/open011_handoff_verification.json"
    )

    acceptance_path = (
        root
        / "artifacts/v82-open011-hud/candidate10-acceptance/acceptance.json"
    )
    current_acceptance_path = args.current_acceptance or (
        root
        / "artifacts/v82-open011-hud/current-review-20260815/acceptance.json"
    )
    source_path = (
        root
        / "tools/recompone-reference/RecompOne.Runtime/Gpu/Enhanced/"
        "EnhancedGlBackend.cs"
    )
    proof_path = (
        root / "artifacts/v82-open011-hud/open011_guest_target_hud_before_after.png"
    )
    notes_path = root / "TO-DO.MD"
    review_path = root / "notes/v82_open011_review_packet.md"
    deployed_paths = {
        "ps1_game_v82": root / "PS1 game/Vigilante82PC.exe",
        "loose_v82": root / "V8_2_LOOSE/Vigilante82PC.exe",
    }

    failures: list[str] = []
    evidence: dict[str, Any] = {}

    acceptance = json.loads(acceptance_path.read_text(encoding="utf-8"))
    acceptance_checks = {
        "passed": acceptance.get("passed") is True,
        "executable_hash": acceptance.get("executable_sha256") == EXPECTED_SHA256,
        "captures_all_guests": acceptance.get("captures") == EXPECTED_GUESTS,
        "traversed_all_guests": acceptance.get("traversed_guests") == EXPECTED_GUESTS,
        "unique_preview_frames": acceptance.get("unique_preview_frames") == 12,
        "quantity_text_bound": acceptance.get("max_quantity_band_white_pixels", 999999) < 1600,
        "one_guest_npc_object": len(acceptance.get("npc_objects", [])) == 1,
        "gameplay_reached": acceptance.get("gameplay") is True,
        "clean_exit": acceptance.get("clean_exit") is True,
        "hud_backing_geometry": ["74", "90"] in acceptance.get("hud_status_geometry", []),
        "target_icon_geometry": ["76", "40"] in acceptance.get("hud_target_icon_geometry", []),
    }
    for name, passed in acceptance_checks.items():
        if not passed:
            failures.append(f"acceptance check failed: {name}")
    evidence["acceptance"] = {
        "path": str(acceptance_path),
        "checks": acceptance_checks,
        "hud_status_geometry": acceptance.get("hud_status_geometry"),
        "hud_target_icon_geometry": acceptance.get("hud_target_icon_geometry"),
        "npc_objects": acceptance.get("npc_objects"),
    }

    current_acceptance = json.loads(
        current_acceptance_path.read_text(encoding="utf-8")
    )
    current_checks = {
        "passed": current_acceptance.get("passed") is True,
        "executable_hash": current_acceptance.get("executable_sha256") == EXPECTED_SHA256,
        "captures_all_guests": current_acceptance.get("captures") == EXPECTED_GUESTS,
        "traversed_all_guests": current_acceptance.get("traversed_guests") == EXPECTED_GUESTS,
        "unique_preview_frames": current_acceptance.get("unique_preview_frames") == 12,
        "quantity_text_bound": current_acceptance.get("max_quantity_band_white_pixels", 999999) < 1600,
        "one_guest_npc_object": len(current_acceptance.get("npc_objects", [])) == 1,
        "gameplay_reached": current_acceptance.get("gameplay") is True,
        "clean_exit": current_acceptance.get("clean_exit") is True,
        "hud_backing_geometry": ["74", "90"] in current_acceptance.get("hud_status_geometry", []),
        "target_icon_geometry": ["76", "40"] in current_acceptance.get("hud_target_icon_geometry", []),
    }
    for name, passed in current_checks.items():
        if not passed:
            failures.append(f"current acceptance check failed: {name}")
    evidence["current_acceptance"] = {
        "path": str(current_acceptance_path),
        "checks": current_checks,
        "hud_status_geometry": current_acceptance.get("hud_status_geometry"),
        "hud_target_icon_geometry": current_acceptance.get("hud_target_icon_geometry"),
        "npc_objects": current_acceptance.get("npc_objects"),
        "elapsed_seconds": current_acceptance.get("elapsed_seconds"),
    }

    deployed_hashes = {
        name: sha256(path)
        for name, path in deployed_paths.items()
    }
    for name, digest in deployed_hashes.items():
        if digest != EXPECTED_SHA256:
            failures.append(f"{name} hash {digest} != {EXPECTED_SHA256}")
    candidate_exe = Path(acceptance["executable"])
    candidate_hash = sha256(candidate_exe)
    if candidate_hash != EXPECTED_SHA256:
        failures.append(f"candidate10 hash {candidate_hash} != {EXPECTED_SHA256}")
    evidence["executables"] = {
        "expected_sha256": EXPECTED_SHA256,
        "deployed": deployed_hashes,
        "candidate10": {
            "path": str(candidate_exe),
            "sha256": candidate_hash,
        },
    }

    source = source_path.read_text(encoding="utf-8")
    status_block = find_status_block(source)
    required_source = [
        "Guest target banks replace the packet's UV, page, and CLUT",
        "r.X == 80 && statusLocalY == 20f",
        "r.W == 84 && r.H == 34",
        "f.Textured && f.RawTexture && f.SemiTrans",
        "f.Material is HleMaterialKind.Ui or HleMaterialKind.ScreenEffect",
        "drawX -= 6f;",
        "drawW += 6;",
        "drawU -= 6;",
    ]
    source_ok, source_missing = has_all(source, required_source)
    volatile_terms = ["f.TPage", "f.Clut", "resolvedU", "resolvedV"]
    volatile_in_status_identity = [
        term for term in volatile_terms if term in status_block
    ]
    if not source_ok:
        failures.append(f"classifier source missing: {source_missing}")
    if volatile_in_status_identity:
        failures.append(
            "status backing classifier still depends on volatile texture state: "
            + ", ".join(volatile_in_status_identity)
        )
    evidence["classifier"] = {
        "path": str(source_path),
        "required_fragments_present": source_ok,
        "missing_fragments": source_missing,
        "volatile_texture_terms_in_status_identity": volatile_in_status_identity,
    }

    proof_dimensions = png_size(proof_path)
    if proof_path.stat().st_size <= 0:
        failures.append("visual proof is empty")
    evidence["visual_proof"] = {
        "path": str(proof_path),
        "bytes": proof_path.stat().st_size,
        "size": proof_dimensions,
        "sha256": sha256(proof_path),
    }

    notes = notes_path.read_text(encoding="utf-8")
    notes_checks = {
        "open011_unchecked": "- [ ] **V82-OPEN-011" in notes,
        "current_hash_recorded": EXPECTED_SHA256 in notes,
        "candidate10_recorded": "candidate10-acceptance" in notes,
        "current_acceptance_recorded": "current-review-20260815" in notes,
        "hud_backing_recorded": "x=74 width=90" in notes,
        "target_icon_recorded": "x=76 width=40" in notes,
        "signoff_pending": "explicit signoff" in notes,
        "v82_executable_boundary": (
            "Vigilante82PC.exe" in notes
            and "Vigilante8PC.exe" in notes
            and "not the acceptance binary for OPEN-011" in notes
        ),
    }
    for name, passed in notes_checks.items():
        if not passed:
            failures.append(f"notes check failed: {name}")
    evidence["notes"] = {
        "path": str(notes_path),
        "checks": notes_checks,
    }

    review = review_path.read_text(encoding="utf-8")
    review_checks = {
        "ready_for_user_review": "READY FOR USER REVIEW" in review,
        "explicit_signoff_required": "explicit" in review and "signoff" in review,
        "candidate8_history_preserved": "candidate8" in review,
        "candidate10_baseline_recorded": "candidate10" in review,
        "candidate_acceptance_recorded": "candidate10-acceptance/acceptance.json" in review,
        "current_acceptance_recorded": "current-review-20260815/acceptance.json" in review,
        "expected_hash_recorded": EXPECTED_SHA256 in review,
        "v82_review_binary_named": "Vigilante82PC.exe" in review,
        "dreamland_boundary_recorded": (
            "Vigilante8PC.exe" in review and "Dreamland" in review
        ),
        "hud_geometry_recorded": (
            "`x=74`, `width=90`" in review
            and "`x=76`, `width=40`" in review
        ),
        "visual_proof_recorded": "open011_guest_target_hud_before_after.png" in review,
        "recheck_command_recorded": "verify_open011_hud_handoff.py" in review,
    }
    for name, passed in review_checks.items():
        if not passed:
            failures.append(f"review packet check failed: {name}")
    evidence["review_packet"] = {
        "path": str(review_path),
        "checks": review_checks,
    }

    stale_logs = sorted((root / "PS1 game").glob("v8*.log"))
    if stale_logs:
        failures.append(
            "PS1 game contains stale v8*.log files: "
            + ", ".join(path.name for path in stale_logs)
        )
    evidence["stale_logs"] = [str(path) for path in stale_logs]

    requirement_checks = {
        "resume_history_bound_to_open011": (
            review_checks["candidate8_history_preserved"]
            and review_checks["candidate10_baseline_recorded"]
            and notes_checks["open011_unchecked"]
        ),
        "complete_enemy_status_backing_restored": (
            acceptance_checks["hud_backing_geometry"]
            and current_checks["hud_backing_geometry"]
            and acceptance_checks["target_icon_geometry"]
            and current_checks["target_icon_geometry"]
        ),
        "candidate10_baseline_verified": (
            acceptance_checks["passed"]
            and candidate_hash == EXPECTED_SHA256
            and review_checks["candidate10_baseline_recorded"]
        ),
        "authored_geometry_material_classifier_verified": (
            source_ok and not volatile_in_status_identity
        ),
        "deployed_vigilante82_hash_verified": all(
            digest == EXPECTED_SHA256 for digest in deployed_hashes.values()
        ),
        "acceptance_artifacts_verified": (
            all(acceptance_checks.values()) and all(current_checks.values())
        ),
        "visual_proof_verified": (
            proof_path.stat().st_size > 0 and proof_dimensions == [2080, 360]
        ),
        "carried_to_user_review_signoff": (
            review_checks["ready_for_user_review"]
            and review_checks["explicit_signoff_required"]
            and notes_checks["signoff_pending"]
        ),
        "dreamland_executable_not_confused": (
            notes_checks["v82_executable_boundary"]
            and review_checks["dreamland_boundary_recorded"]
        ),
    }
    for name, passed in requirement_checks.items():
        if not passed:
            failures.append(f"objective requirement check failed: {name}")

    report = {
        "schema": "v82.open011-hud-handoff.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "objective_requirements": requirement_checks,
        "evidence": evidence,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
