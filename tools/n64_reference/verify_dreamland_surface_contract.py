#!/usr/bin/env python3
"""Verify the source-to-renderer contract for Dreamland's road and water.

This is a static companion to the runtime framebuffer/RSP/RDP harness.  It
proves that the converted arena still carries the authored runtime route
surface, that the recovered loader terrain-conforms its two edge vertices, and
that the Enhanced renderer reserves depth occlusion for explicitly identified
water/glass rather than applying it to every semitransparent world primitive.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path


def be16(data: bytes, offset: int, *, signed: bool = False) -> int:
    return int.from_bytes(data[offset : offset + 2], "big", signed=signed)


def be32(data: bytes, offset: int, *, signed: bool = False) -> int:
    return int.from_bytes(data[offset : offset + 4], "big", signed=signed)


def walk_iff(
    data: bytes,
    start: int = 0,
    end: int | None = None,
):
    if end is None:
        end = len(data)
    offset = start
    while offset + 8 <= end:
        tag = data[offset : offset + 4]
        size = be32(data, offset + 4)
        body = offset + 8
        chunk_end = body + size
        if chunk_end > end or chunk_end > len(data):
            raise ValueError(
                f"chunk {tag!r} at 0x{offset:X} exceeds its container"
            )
        if tag == b"FORM":
            if size < 4:
                raise ValueError(f"short FORM at 0x{offset:X}")
            form_type = data[body : body + 4]
            yield offset, form_type, data[body + 4 : chunk_end]
            yield from walk_iff(data, body + 4, chunk_end)
        else:
            yield offset, tag, data[body:chunk_end]
        offset += 8 + ((size + 1) & ~1)


def route_records(data: bytes) -> tuple[list[dict[str, int]], list[dict[str, int]]]:
    xrtp: list[dict[str, int]] = []
    rseg: list[dict[str, int]] = []
    for offset, tag, payload in walk_iff(data):
        if tag == b"XRTP":
            if len(payload) < 12:
                raise ValueError(f"short XRTP at 0x{offset:X}")
            xrtp.append(
                {
                    "offset": offset,
                    "width16": be32(payload, 0, signed=True),
                    "step16": be32(payload, 4, signed=True),
                    "texture_id": be16(payload, 8, signed=True),
                    "flags": be16(payload, 10),
                    "payload_bytes": len(payload) - 12,
                }
            )
        elif tag == b"RSEG":
            if len(payload) < 4:
                raise ValueError(f"short RSEG at 0x{offset:X}")
            rseg.append(
                {
                    "offset": offset,
                    "route_type": be16(payload, 0, signed=True),
                    "order": be16(payload, 2, signed=True),
                }
            )
    return xrtp, rseg


def load_json(path: Path) -> dict[str, object]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"{path} does not contain a JSON object")
    return value


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    artifacts = root / "artifacts"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--arena",
        type=Path,
        default=artifacts / "n64_reference" / "static" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--n64-arena",
        type=Path,
        default=artifacts / "n64_reference" / "static" / "DREAMLND_N64.EXP",
    )
    parser.add_argument(
        "--rdp-texture-state",
        type=Path,
        default=(
            artifacts
            / "n64_reference"
            / "dreamland-rdp-texture-state-proof-20260814.json"
        ),
    )
    parser.add_argument(
        "--loader",
        type=Path,
        default=root / "analysis" / "dll" / "LOAD" / "decomp" / "80102bd4.c",
    )
    parser.add_argument(
        "--renderer",
        type=Path,
        default=(
            root
            / "tools"
            / "recompone-reference"
            / "RecompOne.Runtime"
            / "Gpu"
            / "Enhanced"
            / "EnhancedGlBackend.cs"
        ),
    )
    parser.add_argument(
        "--rdp-audit",
        type=Path,
        default=artifacts / "n64_reference" / "dreamland_rdp_state.json",
    )
    parser.add_argument(
        "--stable-candidate",
        type=Path,
        default=(
            artifacts
            / "v8-dreamland-road-depth-ab"
            / "20260803-painter-depth-candidate-2"
            / "runtime-stability.json"
        ),
    )
    parser.add_argument(
        "--water-protected",
        type=Path,
        default=(
            artifacts
            / "v8-dreamland-road-depth-ab"
            / "20260803-painter-depth-candidate-2"
            / "water-protected"
            / "verification.json"
        ),
    )
    parser.add_argument(
        "--water-negative-control",
        type=Path,
        default=(
            artifacts
            / "v8-dreamland-road-depth-ab"
            / "20260803-painter-depth-candidate-2"
            / "water-unprotected"
            / "verification.json"
        ),
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=(
            artifacts
            / "n64_reference"
            / "dreamland_surface_contract.json"
        ),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    failures: list[str] = []

    arena = args.arena.read_bytes()
    xrtp, rseg = route_records(arena)
    n64_xrtp, n64_rseg = route_records(args.n64_arena.read_bytes())
    route_type_counts = Counter(record["route_type"] for record in rseg)
    used_types = sorted(route_type_counts)
    if len(xrtp) != 2:
        failures.append(f"expected 2 XRTP descriptors, found {len(xrtp)}")
    if len(rseg) != 35:
        failures.append(f"expected 35 RSEG records, found {len(rseg)}")
    if any(route_type < 0 or route_type >= len(xrtp) for route_type in used_types):
        failures.append(f"RSEG references invalid XRTP types: {used_types}")

    primary_route = xrtp[0] if xrtp else {}
    primary_flags = int(primary_route.get("flags", 0))
    if not primary_flags & 0x0002:
        failures.append("primary XRTP is not a textured POLY_GT4 route")
    if primary_flags & 0x0100:
        failures.append("converted primary XRTP still carries false PS1 semitransparency")
    if int(primary_route.get("width16", 0)) <= 0:
        failures.append("primary XRTP has no positive authored width")
    if int(primary_route.get("step16", 0)) <= 0:
        failures.append("primary XRTP has no positive authored tessellation step")
    if route_type_counts.get(0, 0) <= 0:
        failures.append("no RSEG uses the primary XRTP descriptor")

    n64_primary_flags = int(n64_xrtp[0].get("flags", 0)) if n64_xrtp else 0
    if not n64_primary_flags & 0x0100:
        failures.append("N64 source primary XRTP no longer carries source bit 0x0100")
    if len(n64_rseg) != len(rseg):
        failures.append("converted RSEG count differs from N64 source")

    texture_state = load_json(args.rdp_texture_state)
    source_route_states = [
        state for state in texture_state.get("triangle_states", [])
        if isinstance(state, dict) and state.get("source_xrtp_index") == 0
    ]
    source_route_triangles = sum(int(state.get("triangles", 0)) for state in source_route_states)
    if source_route_triangles <= 0:
        failures.append("runtime RDP trace contains no triangles tied to source XRTP 0")
    for state in source_route_states:
        mode = state.get("render_mode", {})
        if (
            not isinstance(mode, dict)
            or not mode.get("z_compare")
            or not mode.get("z_update")
            or mode.get("z_mode") != "opaque"
        ):
            failures.append("source XRTP 0 reaches a non-opaque/non-Z-writing RDP state")

    loader = args.loader.read_text(encoding="utf-8", errors="replace")
    terrain_height_calls = loader.count("Terrain_HeightAt")
    if terrain_height_calls < 3:
        failures.append(
            "recovered route builder does not show center plus both edge "
            f"Terrain_HeightAt sampling (found {terrain_height_calls} calls)"
        )

    renderer = args.renderer.read_text(encoding="utf-8", errors="replace")
    renderer_assertions = {
        "generic_world_transparent_predicate_removed":
            "bool worldTransparent" not in renderer,
        "water_requires_explicit_provenance":
            "f.DreamlandWater &&\n            coherentRasterDepth" in renderer,
        "only_water_or_glass_enters_transparent_depth":
            "(sourceOccludedTransparent || glassTransparent)" in renderer,
        "n64_route_uses_source_depth_compare":
            "f.N64RouteDepthCompare &&" in renderer and
            "sourceOpaqueDepthTest ||" in renderer,
        "n64_route_uses_single_pass_depth_compare_write":
            "Begin(f, 3, depthTest, depthWrite, sourceDepthCompareWrite)" in renderer and
            "if (_kDepthWrite && !_kSourceDepthCompareWrite)" in renderer,
        "painter_transparency_is_traced":
            "painter-transparent=" in renderer,
        "unexpected_transparent_depth_is_traced":
            "unexpected-transparent-depth=" in renderer,
    }
    for name, passed in renderer_assertions.items():
        if not passed:
            failures.append(f"renderer assertion failed: {name}")

    rdp_audit = load_json(args.rdp_audit)
    rdp_summary = rdp_audit.get("summary", {})
    if not isinstance(rdp_summary, dict):
        failures.append("N64 RDP audit has no summary")
        rdp_summary = {}
    if rdp_summary.get("triangles") != 2119:
        failures.append(
            "N64 authored-XOBF triangle census changed: "
            f"{rdp_summary.get('triangles')}"
        )
    if rdp_summary.get("chunk_counts", {}).get("RSEG") != 35:
        failures.append("N64 RDP audit does not retain all 35 RSEG records")

    stable = load_json(args.stable_candidate)
    max_luma_jump = stable.get("max_adjacent_road_luma_jump")
    max_temporal_residual = stable.get("max_road_temporal_residual")
    if stable.get("status") != "PASS":
        failures.append(
            "retained runtime candidate did not pass its dedicated stability gate"
        )
    if (
        not isinstance(max_temporal_residual, (int, float))
        or max_temporal_residual > 1.5
    ):
        failures.append(
            "retained runtime candidate road residual is not stable: "
            f"{max_temporal_residual}"
        )

    protected = load_json(args.water_protected)
    protected_trace = protected.get("trace_summary", {})
    if protected.get("status") != "PASS":
        failures.append("protected-water trace did not pass")
    if not isinstance(protected_trace, dict):
        failures.append("protected-water trace has no summary")
        protected_trace = {}
    if protected_trace.get("leak_samples") != 0:
        failures.append("protected-water trace contains leaked samples")
    if int(protected_trace.get("occluded_samples", 0)) <= 0:
        failures.append("protected-water trace did not exercise occlusion")
    if protected_trace.get("depth_enabled_batches") != protected_trace.get("batches"):
        failures.append("not every protected water batch used depth")

    negative = load_json(args.water_negative_control)
    negative_trace = negative.get("trace_summary", {})
    if negative.get("status") != "PASS":
        failures.append("water negative control did not pass")
    if not isinstance(negative_trace, dict):
        failures.append("water negative control has no summary")
        negative_trace = {}
    if int(negative_trace.get("leak_samples", 0)) <= 0:
        failures.append("water negative control exposed no leaks")
    if negative_trace.get("depth_disabled_batches") != negative_trace.get("batches"):
        failures.append("water negative control did not disable every batch")

    report = {
        "schema": "v8.dreamland-surface-contract.v2",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "arena": str(args.arena.resolve()),
        "route": {
            "xrtp": xrtp,
            "n64_source_xrtp": n64_xrtp,
            "rseg_count": len(rseg),
            "route_type_counts": {
                str(key): value for key, value in sorted(route_type_counts.items())
            },
            "terrain_height_calls_in_recovered_builder": terrain_height_calls,
            "runtime_source_xrtp0_triangles": source_route_triangles,
            "runtime_source_xrtp0_states": source_route_states,
        },
        "n64_authored_xobf": {
            "triangles": rdp_summary.get("triangles"),
            "unique_triangle_states": rdp_summary.get("unique_triangle_states"),
            "note": (
                "The RDP audit covers authored XOBF display lists. The road is "
                "instead constructed from RSEG/XRTP by runtime code."
            ),
        },
        "renderer_assertions": renderer_assertions,
        "retained_runtime_evidence": {
            "stability_status": stable.get("status"),
            "max_adjacent_road_luma_jump": max_luma_jump,
            "max_road_temporal_residual": max_temporal_residual,
            "water_occluded_samples": protected_trace.get("occluded_samples"),
            "water_leak_samples": protected_trace.get("leak_samples"),
            "negative_control_leak_samples": negative_trace.get("leak_samples"),
        },
    }
    encoded = json.dumps(report, indent=2)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(encoded + "\n", encoding="utf-8")
    print(encoded)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
