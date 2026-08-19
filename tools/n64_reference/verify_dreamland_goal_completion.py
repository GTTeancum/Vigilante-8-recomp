#!/usr/bin/env python3
"""Audit every requirement of the Super Dreamland spawn-road goal."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image


EXPECTED_ARENA_SHA256 = (
    "8000D187F5409F7AB225957E37153ECA678A074FA4509BA47FAA9FDFEFB23917"
)


def load(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--combined", type=Path, required=True)
    parser.add_argument("--contract", type=Path, required=True)
    parser.add_argument("--repeat", type=Path, required=True)
    parser.add_argument("--normal-play", type=Path, required=True)
    parser.add_argument("--build-reproducibility", type=Path, required=True)
    parser.add_argument("--run-manifest", type=Path, required=True)
    parser.add_argument("--progress-log", type=Path, required=True)
    parser.add_argument("--deployed-exe", type=Path, required=True)
    parser.add_argument("--tested-exe", type=Path, required=True)
    parser.add_argument("--rebuilt-exe", type=Path, required=True)
    parser.add_argument("--deployed-arena", type=Path, required=True)
    parser.add_argument("--side-by-side", type=Path, required=True)
    parser.add_argument("--visual-board", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    combined = load(args.combined)
    contract = load(args.contract)
    repeat = load(args.repeat)
    normal = load(args.normal_play)
    build_reproducibility = load(args.build_reproducibility)
    manifest = load(args.run_manifest)
    progress = args.progress_log.read_text(encoding="utf-8", errors="replace")
    checks: list[dict[str, object]] = []
    failures: list[str] = []

    def check(name: str, passed: bool, evidence: object) -> None:
        checks.append({"name": name, "passed": bool(passed), "evidence": evidence})
        if not passed:
            failures.append(name)

    reference = combined["reference_run"]
    sync = reference["synchronization"]
    reference_repeat = reference["repeat_determinism"]
    rsp_tasks = reference["rsp_task_determinism"]
    route_memory = reference["route_rdram_determinism"]
    check(
        "deterministic hidden targeted N64 harness",
        reference["hidden_desktop"]
        and manifest["hidden_desktop"]
        and manifest["profile"] == "angrylion-cxd4"
        and manifest["audio_mode"] == "null"
        and reference_repeat["status"] == "PASS"
        and reference_repeat["identical_frames"] == 14
        and rsp_tasks["graphics_tasks_byte_identical"]
        and route_memory["status"] == "PASS"
        and all(region["identical"] for region in route_memory["regions"]),
        {
            "profile": manifest["profile"],
            "audio_mode": manifest["audio_mode"],
            "identical_reference_frames": reference_repeat["identical_frames"],
            "graphics_tasks": rsp_tasks["graphics_tasks"],
            "whole_rdram_audio_only_different_bytes": route_memory[
                "whole_rdram_different_bytes"
            ],
        },
    )
    frames = sync["frames"]
    check(
        "synchronized framebuffer and RSP/RDP evidence",
        sync["status"] == "PASS"
        and reference["framebuffer_captures"] == 14
        and reference["dmem_rdram_pairs"] == 14
        and sync["synchronized_frames"] == 14
        and sync["total_rdp_spans"] == 9371
        and len(frames) == 14
        and all(frame["status"] == "PASS" for frame in frames)
        and all(frame["framebuffer_size"] == [640, 480] for frame in frames),
        {
            "framebuffers": reference["framebuffer_captures"],
            "memory_pairs": reference["dmem_rdram_pairs"],
            "rdp_spans": sync["total_rdp_spans"],
            "rsp_trace_sha256": sync["rsp_trace_sha256"],
            "rdp_trace_sha256": sync["rdp_trace_sha256"],
        },
    )

    source = combined["source_geometry"]
    runtime = combined["runtime_geometry"]
    check(
        "source and submitted geometry determined",
        len(source["n64_xrtp"]) == 2
        and source["rseg_count"] == 35
        and source["route_type_counts"] == {"0": 20, "1": 15}
        and runtime["triangles"] == 102
        and runtime["unique_vertices"] == 108
        and runtime["bbox"]["y"][0] < runtime["bbox"]["y"][1],
        {
            "xrtp_descriptors": len(source["n64_xrtp"]),
            "rseg_count": source["rseg_count"],
            "frame_5460_triangles": runtime["triangles"],
            "frame_5460_unique_vertices": runtime["unique_vertices"],
            "frame_5460_bounds": runtime["bbox"],
        },
    )
    order = combined["submission_order"]
    check(
        "submission order determined",
        order["road_draw_events"] == 481
        and order["road_triangles"] == 962
        and order["first_road_event"]["frame"] < order["last_road_event"]["frame"]
        and order["first_road_event"]["command_index"]
        < order["last_road_event"]["command_index"],
        {
            "road_draw_events": order["road_draw_events"],
            "road_triangles": order["road_triangles"],
            "first": order["first_road_event"],
            "last": order["last_road_event"],
        },
    )
    render_mode = combined["rdp_depth_render_state"][0]["render_mode"]
    check(
        "RDP depth/write/render mode determined",
        render_mode["z_compare"]
        and render_mode["z_update"]
        and render_mode["z_mode"] == "opaque"
        and not render_mode["force_blend"],
        combined["rdp_depth_render_state"][0],
    )
    alteration = combined["runtime_code_alters_geometry"]
    check(
        "runtime geometry alteration determined",
        alteration["terrain_height_calls_in_builder"] == 3
        and alteration["runtime_y_varies"],
        alteration,
    )

    source_flags = source["n64_xrtp"][0]["flags"]
    converted_flags = source["converted_xrtp"][0]["flags"]
    arena_hash = sha256(args.deployed_arena)
    check(
        "converted original-V8 Dreamland road corrected",
        source_flags == 0x192
        and converted_flags == 0x92
        and source_flags - converted_flags == 0x100
        and arena_hash == EXPECTED_ARENA_SHA256,
        {
            "source_flags": f"0x{source_flags:04X}",
            "converted_flags": f"0x{converted_flags:04X}",
            "deployed_arena_sha256": arena_hash,
        },
    )
    renderer_assertions = contract["renderer_assertions"]
    combined_renderer = combined["renderer_contract"]
    check(
        "Enhanced renderer corrected to source-scoped single-pass depth",
        contract["status"] == "PASS"
        and not contract["failures"]
        and all(renderer_assertions.values())
        and all(combined_renderer.values()),
        {
            "surface_contract": contract["status"],
            "assertions": renderer_assertions,
            "retained_runtime_evidence": contract["retained_runtime_evidence"],
        },
    )

    port = combined["port_trace"]
    route_triangles = port["route_triangles"]
    check(
        "source-tagged port traces prove corrected behavior",
        route_triangles > 0
        and port["route_depth_test_triangles"] == route_triangles
        and port["route_depth_write_triangles"] == route_triangles
        and port["route_depth_compare_write_triangles"] == route_triangles
        and port["route_transparent_triangles"] == 0
        and port["route_packet_word_counts"].get("12", 0) > 0,
        {
            "route_triangles": route_triangles,
            "single_pass_depth_compare_write": port[
                "route_depth_compare_write_triangles"
            ],
            "transparent": port["route_transparent_triangles"],
            "native_12_word_packets": port["route_packet_word_counts"].get(
                "12", 0
            ),
        },
    )
    check(
        "deterministic corrected-port repeat",
        repeat["status"] == "PASS"
        and not repeat["failures"]
        and repeat["route_intervals_identical"]
        and repeat["positive_tick_color_summaries_identical"]
        and repeat["deterministic_rng_seeds"] == [1]
        and repeat["exact_capture_frames"] == 20
        and repeat["compared_capture_frames"] == 20
        and repeat["max_whole_mean_abs_rgb"] == 0.0
        and repeat["max_road_mean_abs_rgb"] == 0.0,
        {
            "seed": repeat["deterministic_rng_seeds"],
            "exact_frames": repeat["exact_capture_frames"],
            "whole_mean_abs_rgb": repeat["max_whole_mean_abs_rgb"],
            "road_mean_abs_rgb": repeat["max_road_mean_abs_rgb"],
        },
    )
    check(
        "proof hook dormant in normal play",
        normal["status"] == "PASS"
        and not normal["failures"]
        and normal["deterministic_seed_log_lines"] == 0
        and normal["clean_match_exit"]
        and normal["last_gameplay_tick"] >= 360,
        normal,
    )

    side_size = list(Image.open(args.side_by_side).size)
    board_size = list(Image.open(args.visual_board).size)
    check(
        "stable side-by-side and visual proof artifacts",
        combined["port_stability"]["status"] == "PASS"
        and Path(port["side_by_side"]).resolve() == args.side_by_side.resolve()
        and side_size[0] > 0
        and side_size[1] > 0
        and board_size == [2400, 1770],
        {
            "port_stability": combined["port_stability"]["status"],
            "side_by_side_size": side_size,
            "visual_board_size": board_size,
            "side_by_side_sha256": sha256(args.side_by_side),
            "visual_board_sha256": sha256(args.visual_board),
        },
    )
    deployed_hash = sha256(args.deployed_exe)
    tested_hash = sha256(args.tested_exe)
    rebuilt_hash = sha256(args.rebuilt_exe)
    build_order = build_reproducibility["independent_generation_order"]
    build_executables = build_reproducibility["executables"]
    check(
        "two end-to-end CUE/RecompOne builds and deployment are byte-identical",
        build_reproducibility["status"] == "PASS"
        and not build_reproducibility["failures"]
        and len(build_reproducibility["cue"]["tracks"]) == 13
        and not build_reproducibility["cue"]["missing_tracks"]
        and build_order["first_build_predates_second_generated_sources"]
        and build_order["second_generated_sources_predate_second_build"]
        and all(
            check["exit_code"] == 0
            for check in build_reproducibility["generated_sources"][
                "post_patch_checks"
            ]
        )
        and deployed_hash
        == tested_hash
        == rebuilt_hash
        == normal["tested_exe_sha256"]
        == build_executables["first_end_to_end"]["sha256"]
        == build_executables["second_end_to_end"]["sha256"]
        == build_executables["deployed"]["sha256"],
        {
            "deployed_exe_sha256": deployed_hash,
            "tested_end_to_end_exe_sha256": tested_hash,
            "repeated_end_to_end_exe_sha256": rebuilt_hash,
            "cue_tracks": len(build_reproducibility["cue"]["tracks"]),
            "generated_sources_sha256": build_reproducibility[
                "generated_sources"
            ]["sha256"],
            "independent_generation_order": build_order,
        },
    )
    check(
        "targeted tracing sufficient without full N64Recomp",
        manifest["profile"] == "angrylion-cxd4"
        and "no full N64Recomp was started" in progress,
        {
            "reference_profile": manifest["profile"],
            "full_n64recomp_started": False,
        },
    )

    report = {
        "schema": "v8.n64-dreamland-spawn-road-goal-audit.v1",
        "status": "PASS" if not failures else "FAIL",
        "goal_state": "READY_FOR_USER_SIGNOFF" if not failures else "INCOMPLETE",
        "failures": failures,
        "checks": checks,
        "input_sha256": {
            "combined": sha256(args.combined),
            "contract": sha256(args.contract),
            "repeat": sha256(args.repeat),
            "normal_play": sha256(args.normal_play),
            "build_reproducibility": sha256(args.build_reproducibility),
            "run_manifest": sha256(args.run_manifest),
        },
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
