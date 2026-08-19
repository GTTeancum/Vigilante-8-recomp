#!/usr/bin/env python3
"""Verify Dreamland's source-derived fog profile from capture through runtime."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
EXPECTED_RGBA = [254, 200, 127, 255]
EXPECTED_MULTIPLIER = 9846
EXPECTED_DISPLACEMENT = -9550
EXPECTED_MINIMUM = 984.9685
EXPECTED_MAXIMUM = 997.9687


def close(actual: float, expected: float) -> bool:
    return abs(actual - expected) < 0.0001


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-rdp", type=Path, required=True)
    parser.add_argument("--projection-proof", type=Path, required=True)
    parser.add_argument("--atlas-proof", type=Path, required=True)
    parser.add_argument("--shader", type=Path, required=True)
    parser.add_argument("--runtime-log", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    source = json.loads(args.source_rdp.read_text(encoding="utf-8"))
    host = source.get("layouts", {}).get("host-word", {})
    setups = host.get("fog_setups", [])
    matching_setups = [
        setup for setup in setups
        if setup.get("rgba") == EXPECTED_RGBA
        and setup.get("multiplier") == EXPECTED_MULTIPLIER
        and setup.get("displacement") == EXPECTED_DISPLACEMENT
        and close(float(setup.get("minimum_per_mille", 0)), EXPECTED_MINIMUM)
        and close(float(setup.get("maximum_per_mille", 0)), EXPECTED_MAXIMUM)
    ]

    projection = json.loads(
        args.projection_proof.read_text(encoding="utf-8")
    )
    projection_exact = (
        projection.get("status") == "PASS"
        and close(float(projection.get("projection_a", 0)), 1.014373779296875)
        and close(float(projection.get("projection_b", 0)), -1.0071868896484375)
        and projection.get("converted_scale_shift") == 8
        and close(float(projection.get("converted_fog_start_view_z", 0)), 5802.401151610324)
        and close(float(projection.get("converted_fog_end_view_z", 0)), 13985.411689547065)
    )

    atlas = json.loads(args.atlas_proof.read_text(encoding="utf-8"))
    palette_exact = (
        atlas.get("status") == "PASS"
        and atlas.get("converted_vs_decoded_source_mismatches") == 0
        and atlas.get("palette_rgb555_mismatches") == 0
        and atlas.get("source_palette_rgb555_sha256")
        == atlas.get("converted_palette_rgb555_sha256")
    )

    shader = args.shader.read_text(encoding="utf-8")
    shader_profile = (
        "uDreamlandN64Fog" in shader
        and "projectionA = 1.014373779296875" in shader
        and "projectionB = -1.0071868896484375 * 256.0" in shader
        and "fogMultiplier = 9846.0" in shader
        and "fogDisplacement = -9550.0" in shader
        and "(projectedZ * fogMultiplier + fogDisplacement) / 256.0" in shader
        and "mix(rgb, uFogColor, amount)" in shader
    )

    runtime = args.runtime_log.read_text(encoding="utf-8", errors="replace")
    profile_match = re.search(
        r"\[EnhancedFog\] source profile DREAMLND "
        r"projectionA=1\.0143737793 projectionB=-1\.0071868896 "
        r"scaleShift=8 factor=9846,-9550 "
        r"mappedViewZ=([0-9.]+)\.\.([0-9.]+)",
        runtime,
    )
    atmosphere_hits = len(re.findall(
        r"\[EnhancedFog\] atmosphere candidate rgb=\(254,200,127\)", runtime
    ))
    selected_profile = profile_match is not None and atmosphere_hits > 0

    failures: list[str] = []
    if not matching_setups:
        failures.append("captured N64 fog setup does not match expected source values")
    if not projection_exact:
        failures.append("captured N64 projection does not produce expected GTE-depth curve")
    if not palette_exact:
        failures.append("converted terrain palette/texels are not source-exact")
    if not shader_profile:
        failures.append("Dreamland shader profile does not contain captured range")
    if not selected_profile:
        failures.append("runtime did not select source profile and captured fog color")

    proof = {
        "schema": "v8.dreamland-fog-profile.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "source_rdp": str(args.source_rdp.resolve()),
        "source_fog_rgba": EXPECTED_RGBA,
        "source_fog_multiplier": EXPECTED_MULTIPLIER,
        "source_fog_displacement": EXPECTED_DISPLACEMENT,
        "source_fog_minimum_per_mille": EXPECTED_MINIMUM,
        "source_fog_maximum_per_mille": EXPECTED_MAXIMUM,
        "source_matching_setup_count": len(matching_setups),
        "source_matching_setup_occurrences": sum(
            int(setup.get("occurrences", 0)) for setup in matching_setups
        ),
        "source_projection_and_depth_mapping_exact": projection_exact,
        "source_projection_proof": str(args.projection_proof.resolve()),
        "converted_palette_and_texels_exact": palette_exact,
        "shader_source_range_present": shader_profile,
        "runtime_source_profile_selected": selected_profile,
        "runtime_source_color_hits": atmosphere_hits,
        "runtime_mapped_view_z": (
            [float(profile_match.group(1)), float(profile_match.group(2))]
            if profile_match else None
        ),
        "runtime_log": str(args.runtime_log.resolve()),
    }
    text = json.dumps(proof, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
