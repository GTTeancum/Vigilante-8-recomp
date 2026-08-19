#!/usr/bin/env python3
"""Verify the focused N64 Super Dreamland spawn-road reference trace."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
from typing import Any


TOOLS = Path(__file__).resolve().parents[1]
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import v8_n64_level as n64  # noqa: E402
from n64_reference.audit_runtime_rdp_shade import (  # noqa: E402
    COMMAND_BYTES,
    TRIANGLES,
    canonicalize,
    render_mode,
)
from n64_reference.audit_runtime_rdp_textures import (  # noqa: E402
    image_descriptor,
    source_xrtp_textures,
)


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    if not isinstance(value, dict):
        raise ValueError(f"{path} does not contain a JSON object")
    return value


def load_jsonl(path: Path) -> list[dict[str, Any]]:
    records: list[dict[str, Any]] = []
    for line_number, line in enumerate(
        path.read_text(encoding="utf-8").splitlines(), 1
    ):
        if not line:
            continue
        value = json.loads(line)
        if not isinstance(value, dict):
            raise ValueError(f"{path}:{line_number} is not a JSON object")
        records.append(value)
    return records


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def ppm_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"{path} is not a binary PPM")
        dimensions = stream.readline().split()
        if len(dimensions) != 2 or stream.readline().strip() != b"255":
            raise ValueError(f"{path} has an invalid PPM header")
        width, height = (int(value) for value in dimensions)
        pixel_offset = stream.tell()
    if path.stat().st_size != pixel_offset + width * height * 3:
        raise ValueError(f"{path} has an invalid PPM payload size")
    return width, height


def capture_synchronization(
    capture: Path,
    capture_frames: Any,
) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    requested: list[int] = []
    try:
        requested = [
            int(token, 0)
            for token in str(capture_frames).split(",")
            if token.strip()
        ]
    except ValueError:
        failures.append(f"invalid capture frame list: {capture_frames!r}")
    if not requested:
        failures.append("reference run requested no explicit capture frames")
    if len(requested) != len(set(requested)):
        failures.append("reference capture frame list contains duplicates")

    rsp_records = load_jsonl(capture / "rsp_rdp_trace.jsonl")
    rdp_records = load_jsonl(capture / "rdp_command_trace.jsonl")
    rsp_by_frame: dict[int, list[dict[str, Any]]] = {}
    rdp_by_frame: dict[int, list[dict[str, Any]]] = {}
    for record in rsp_records:
        rsp_by_frame.setdefault(int(record["frame"]), []).append(record)
    for record in rdp_records:
        rdp_by_frame.setdefault(int(record["frame"]), []).append(record)

    requested_set = set(requested)
    if set(rsp_by_frame) != requested_set:
        failures.append(
            "RSP trace frames differ from requested frames: "
            f"requested={sorted(requested_set)} actual={sorted(rsp_by_frame)}"
        )
    if set(rdp_by_frame) != requested_set:
        failures.append(
            "RDP trace frames differ from requested frames: "
            f"requested={sorted(requested_set)} actual={sorted(rdp_by_frame)}"
        )

    frame_reports: list[dict[str, Any]] = []
    for frame in requested:
        prefix = f"n64_frame_{frame}"
        framebuffer_paths = list(capture.glob(f"{prefix}.ppm"))
        dmem_paths = list(capture.glob(f"{prefix}_dlist_*.dmem.bin"))
        imem_paths = list(capture.glob(f"{prefix}_dlist_*.imem.bin"))
        rdram_paths = list(capture.glob(f"{prefix}_dlist_*.rdram.bin"))
        records = rsp_by_frame.get(frame, [])
        frame_failures: list[str] = []
        for label, paths in (
            ("framebuffer", framebuffer_paths),
            ("DMEM", dmem_paths),
            ("IMEM", imem_paths),
            ("RDRAM", rdram_paths),
        ):
            if len(paths) != 1:
                frame_failures.append(
                    f"expected one {label} artifact, found {len(paths)}"
                )
        if len(records) != 1:
            frame_failures.append(
                f"expected one RSP identity record, found {len(records)}"
            )

        sequence: int | None = None
        rsp_record = records[0] if len(records) == 1 else None
        if rsp_record is not None:
            sequence = int(rsp_record["dlist_sequence"])
            expected_stem = f"{prefix}_dlist_{sequence}"
            for label, paths in (
                ("DMEM", dmem_paths),
                ("IMEM", imem_paths),
                ("RDRAM", rdram_paths),
            ):
                if len(paths) == 1 and not paths[0].name.startswith(expected_stem):
                    frame_failures.append(
                        f"{label} display-list sequence does not match {sequence}"
                    )
            rdram_size = int(rsp_record["rdram_size"])
            if len(dmem_paths) == 1 and dmem_paths[0].stat().st_size != 0x1000:
                frame_failures.append("DMEM dump is not 4096 bytes")
            if len(imem_paths) == 1 and imem_paths[0].stat().st_size != 0x1000:
                frame_failures.append("IMEM dump is not 4096 bytes")
            if len(rdram_paths) == 1 and rdram_paths[0].stat().st_size != rdram_size:
                frame_failures.append(
                    "RDRAM dump size differs from the synchronized RSP record"
                )

        framebuffer_dimensions: list[int] | None = None
        if len(framebuffer_paths) == 1:
            try:
                framebuffer_dimensions = list(ppm_size(framebuffer_paths[0]))
            except ValueError as error:
                frame_failures.append(str(error))

        frame_rdp_records = sorted(
            rdp_by_frame.get(frame, []),
            key=lambda item: int(item["dlist_sequence"]),
        )
        if not frame_rdp_records:
            frame_failures.append("no indexed RDP command spans")
        rdp_digest = hashlib.sha256()
        for record in frame_rdp_records:
            start = int(record["start"])
            end = int(record["end"])
            byte_count = int(record["bytes"])
            filename = (
                f"{prefix}_rdp_{record['dlist_sequence']}_"
                f"{start}_{end}.bin"
            )
            path = capture / filename
            if end - start != byte_count:
                frame_failures.append(
                    f"{filename} metadata byte range is inconsistent"
                )
                continue
            if not path.exists():
                frame_failures.append(f"missing indexed RDP span {filename}")
                continue
            data = path.read_bytes()
            if len(data) != byte_count:
                frame_failures.append(
                    f"{filename} has {len(data)} bytes, expected {byte_count}"
                )
                continue
            rdp_digest.update(filename.encode("ascii"))
            rdp_digest.update(b"\0")
            rdp_digest.update(data)

        failures.extend(
            f"capture frame {frame}: {failure}"
            for failure in frame_failures
        )
        frame_reports.append(
            {
                "frame": frame,
                "status": "PASS" if not frame_failures else "FAIL",
                "rsp_dlist_sequence": sequence,
                "vi_origin": None if rsp_record is None else int(rsp_record["vi_origin"]),
                "vi_width": None if rsp_record is None else int(rsp_record["vi_width"]),
                "framebuffer_size": framebuffer_dimensions,
                "framebuffer_sha256": (
                    sha256_file(framebuffer_paths[0])
                    if len(framebuffer_paths) == 1 else None
                ),
                "dmem_sha256": (
                    sha256_file(dmem_paths[0]) if len(dmem_paths) == 1 else None
                ),
                "imem_sha256": (
                    sha256_file(imem_paths[0]) if len(imem_paths) == 1 else None
                ),
                "rdram_sha256": (
                    sha256_file(rdram_paths[0]) if len(rdram_paths) == 1 else None
                ),
                "rdp_spans": len(frame_rdp_records),
                "rdp_sha256": rdp_digest.hexdigest().upper(),
            }
        )

    return (
        {
            "status": "PASS" if not failures else "FAIL",
            "requested_frames": requested,
            "synchronized_frames": sum(
                item["status"] == "PASS" for item in frame_reports
            ),
            "total_rdp_spans": sum(
                int(item["rdp_spans"]) for item in frame_reports
            ),
            "rsp_trace_sha256": sha256_file(capture / "rsp_rdp_trace.jsonl"),
            "rdp_trace_sha256": sha256_file(capture / "rdp_command_trace.jsonl"),
            "frames": frame_reports,
        },
        failures,
    )


def compare_synchronization(
    primary: dict[str, Any],
    repeat: dict[str, Any],
) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    keys = (
        "rsp_dlist_sequence",
        "vi_origin",
        "vi_width",
        "framebuffer_size",
        "framebuffer_sha256",
        "dmem_sha256",
        "imem_sha256",
        "rdp_spans",
        "rdp_sha256",
    )
    primary_frames = {
        int(item["frame"]): item for item in primary.get("frames", [])
    }
    repeat_frames = {
        int(item["frame"]): item for item in repeat.get("frames", [])
    }
    if set(primary_frames) != set(repeat_frames):
        failures.append(
            "repeat capture frame set differs: "
            f"primary={sorted(primary_frames)} repeat={sorted(repeat_frames)}"
        )

    comparisons: list[dict[str, Any]] = []
    for frame in sorted(set(primary_frames) | set(repeat_frames)):
        first = primary_frames.get(frame)
        second = repeat_frames.get(frame)
        mismatched: list[str] = []
        if first is None or second is None:
            mismatched.append("missing_frame")
        else:
            mismatched = [key for key in keys if first.get(key) != second.get(key)]
        if mismatched:
            failures.append(
                f"repeat capture frame {frame} differs in {','.join(mismatched)}"
            )
        comparisons.append(
            {
                "frame": frame,
                "status": "PASS" if not mismatched else "FAIL",
                "mismatched_fields": mismatched,
                "whole_rdram_identical": (
                    first is not None
                    and second is not None
                    and first.get("rdram_sha256") == second.get("rdram_sha256")
                ),
            }
        )

    return (
        {
            "status": "PASS" if not failures else "FAIL",
            "identical_frames": sum(
                item["status"] == "PASS" for item in comparisons
            ),
            "compared_frames": len(comparisons),
            "fields": list(keys),
            "whole_rdram_note": (
                "Whole RDRAM includes scheduler-sized type-2 audio task data "
                "and is reported diagnostically. Graphics-task and route-"
                "relevant RDRAM determinism are checked separately."
            ),
            "whole_rdram_identical_frames": sum(
                bool(item["whole_rdram_identical"]) for item in comparisons
            ),
            "frames": comparisons,
        },
        failures,
    )


def compare_rsp_tasks(
    primary_capture: Path,
    repeat_capture: Path,
) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    primary = load_jsonl(primary_capture / "rsp_task_trace.jsonl")
    repeat = load_jsonl(repeat_capture / "rsp_task_trace.jsonl")

    def tasks(records: list[dict[str, Any]], task_type: int) -> list[dict[str, Any]]:
        return [record for record in records if int(record.get("task_type", 0)) == task_type]

    primary_graphics = tasks(primary, 1)
    repeat_graphics = tasks(repeat, 1)
    graphics_identical = primary_graphics == repeat_graphics and bool(primary_graphics)
    if not graphics_identical:
        failures.append("repeat RSP graphics task trace differs")

    primary_audio = tasks(primary, 2)
    repeat_audio = tasks(repeat, 2)
    audio_mismatch_fields: dict[str, int] = {}
    differing_audio_tasks = 0
    if len(primary_audio) != len(repeat_audio):
        failures.append(
            "repeat RSP audio task count differs: "
            f"primary={len(primary_audio)} repeat={len(repeat_audio)}"
        )
    for first, second in zip(primary_audio, repeat_audio):
        mismatched = [
            key for key in first
            if first.get(key) != second.get(key)
        ]
        if mismatched:
            differing_audio_tasks += 1
        for key in mismatched:
            audio_mismatch_fields[key] = audio_mismatch_fields.get(key, 0) + 1
    unexpected_audio_fields = sorted(set(audio_mismatch_fields) - {"data_size"})
    if unexpected_audio_fields:
        failures.append(
            "repeat RSP audio tasks differ outside scheduler-sized command data: "
            + ",".join(unexpected_audio_fields)
        )

    return (
        {
            "status": "PASS" if not failures else "FAIL",
            "graphics_tasks": len(primary_graphics),
            "graphics_tasks_byte_identical": graphics_identical,
            "audio_tasks": len(primary_audio),
            "differing_audio_tasks": differing_audio_tasks,
            "audio_mismatch_fields": audio_mismatch_fields,
            "classification": (
                "All graphics task records are byte-identical. Residual task "
                "variance is confined to type-2 audio command-list data_size."
            ),
        },
        failures,
    )


def compare_route_rdram(
    primary_capture: Path,
    repeat_capture: Path,
    synchronization: dict[str, Any],
    repeat_synchronization: dict[str, Any],
    rsp: dict[str, Any],
) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    frame = int(rsp.get("frame", 5460))

    def frame_record(report: dict[str, Any]) -> dict[str, Any] | None:
        return next(
            (
                item for item in report.get("frames", [])
                if int(item["frame"]) == frame
            ),
            None,
        )

    first_record = frame_record(synchronization)
    second_record = frame_record(repeat_synchronization)
    if first_record is None or second_record is None:
        failures.append(f"route RDRAM comparison lacks synchronized frame {frame}")
        return ({"status": "FAIL", "frame": frame, "regions": []}, failures)

    def rdram_path(capture: Path, record: dict[str, Any]) -> Path:
        return capture / (
            f"n64_frame_{frame}_dlist_{record['rsp_dlist_sequence']}.rdram.bin"
        )

    primary = rdram_path(primary_capture, first_record).read_bytes()
    repeat = rdram_path(repeat_capture, second_record).read_bytes()
    if len(primary) != len(repeat):
        failures.append(
            f"route RDRAM sizes differ: primary={len(primary)} repeat={len(repeat)}"
        )

    regions: list[tuple[str, list[tuple[int, int]]]] = []
    task = rsp.get("task_display_list", {})
    task_start = int(str(task.get("start", "0")), 0)
    task_size = int(task.get("size", 0))
    regions.append(("active_rsp_display_list", [(task_start, task_size)]))

    vertex_ranges = sorted(
        {
            (int(str(vertex["address"]), 0), 16)
            for vertex in rsp.get("vertices", [])
            if isinstance(vertex, dict) and "address" in vertex
        }
    )
    regions.append(("referenced_route_vertices", vertex_ranges))

    source_xrtp = rsp.get("source_xrtp", {})
    texture_bytes = int(source_xrtp.get("pixel_bytes", 0))
    texture_ranges = [
        (int(str(address), 0), texture_bytes)
        for address in rsp.get("matched_texture_addresses", [])
    ]
    regions.append(("xrtp0_texture_pixels", texture_ranges))

    region_reports: list[dict[str, Any]] = []
    for name, ranges in regions:
        first_digest = hashlib.sha256()
        second_digest = hashlib.sha256()
        byte_count = 0
        for start, size in ranges:
            byte_count += size
            first_digest.update(primary[start:start + size])
            second_digest.update(repeat[start:start + size])
        first_hash = first_digest.hexdigest().upper()
        second_hash = second_digest.hexdigest().upper()
        identical = bool(ranges) and first_hash == second_hash
        if not identical:
            failures.append(f"repeat route RDRAM differs in {name}")
        region_reports.append(
            {
                "name": name,
                "ranges": len(ranges),
                "bytes": byte_count,
                "identical": identical,
                "sha256": first_hash if identical else None,
                "primary_sha256": first_hash,
                "repeat_sha256": second_hash,
            }
        )

    differing_bytes = sum(
        first != second for first, second in zip(primary, repeat)
    ) + abs(len(primary) - len(repeat))
    return (
        {
            "status": "PASS" if not failures else "FAIL",
            "frame": frame,
            "regions": region_reports,
            "whole_rdram_identical": differing_bytes == 0,
            "whole_rdram_different_bytes": differing_bytes,
        },
        failures,
    )


def route_records(data: bytes) -> tuple[list[dict[str, int]], list[dict[str, Any]]]:
    xrtp: list[dict[str, int]] = []
    rseg: list[dict[str, Any]] = []
    for offset, tag, payload, _parent in n64.iter_chunks(data):
        if tag == b"XRTP":
            if len(payload) < 12:
                raise n64.FormatError(f"short XRTP at 0x{offset:X}")
            xrtp.append(
                {
                    "offset": offset,
                    "width16": n64.be32(payload, 0, signed=True),
                    "step16": n64.be32(payload, 4, signed=True),
                    "texture_id": n64.be16(payload, 8, signed=True),
                    "flags": n64.be16(payload, 10),
                    "payload_bytes": len(payload) - 12,
                }
            )
        elif tag == b"RSEG":
            if len(payload) < 4:
                raise n64.FormatError(f"short RSEG at 0x{offset:X}")
            halfwords = [
                n64.be16(payload, index, signed=True)
                for index in range(0, len(payload) & ~1, 2)
            ]
            rseg.append(
                {
                    "offset": offset,
                    "route_type": halfwords[0],
                    "order": halfwords[1],
                    "raw_halfwords": halfwords,
                }
            )
    return xrtp, rseg


def triangle_count(opcode: int) -> int:
    if opcode in {0x08, 0x0B, 0x0D}:
        return 1
    if opcode in {0x09, 0x0A, 0x0F}:
        return 2
    return 0


def scan_rdp_submission(
    capture: Path,
    arena: bytes,
    rdram: bytes,
    xrtp_index: int,
) -> dict[str, Any]:
    records = [
        json.loads(line)
        for line in (capture / "rdp_command_trace.jsonl")
        .read_text(encoding="utf-8")
        .splitlines()
        if line
    ]
    records.sort(key=lambda item: (int(item["frame"]), int(item["dlist_sequence"])))

    source_textures = source_xrtp_textures(arena, rdram)
    target_addresses = {
        int(value, 16)
        for item in source_textures
        if int(item["xrtp_index"]) == xrtp_index
        for value in item["rdram_addresses"]
    }
    current_image: tuple[int, int, int, int] | None = None
    combiner: tuple[int, int] | None = None
    other_mode_low: tuple[int, int] | None = None
    other_mode_high: tuple[int, int] | None = None
    environment: tuple[int, int, int, int] | None = None
    tile_tmem = [0] * 8
    tmem_images: dict[int, tuple[int, int, int, int]] = {}
    command_index = 0
    triangle_index = 0
    road_events: list[dict[str, Any]] = []
    road_state_counts: Counter[tuple[Any, ...]] = Counter()

    for record in records:
        filename = (
            f"n64_frame_{record['frame']}_rdp_{record['dlist_sequence']}_"
            f"{record['start']}_{record['end']}.bin"
        )
        path = capture / filename
        if not path.exists():
            continue
        data = canonicalize(path.read_bytes())
        cursor = 0
        while cursor + 8 <= len(data):
            word0 = struct.unpack_from(">I", data, cursor)[0]
            opcode = (word0 >> 24) & 0x3F
            length = COMMAND_BYTES.get(opcode, 8)
            if cursor + length > len(data):
                break
            words = struct.unpack_from(f">{length // 4}I", data, cursor)
            if opcode == 0x3D:
                desc = image_descriptor(words)
                current_image = (
                    desc["address"], desc["format"], desc["size"], desc["width"]
                )
            elif opcode == 0x35:
                tile_tmem[(words[1] >> 24) & 7] = words[0] & 0x1FF
            elif opcode in {0x30, 0x33, 0x34}:
                tile = (words[1] >> 24) & 7
                if current_image is not None:
                    tmem_images[tile_tmem[tile]] = current_image
            elif opcode == 0x2F:
                other_mode_low = (words[0] & 0x00FFFFFF, words[1])
            elif opcode == 0x2E:
                other_mode_high = (words[0] & 0x00FFFFFF, words[1])
            elif opcode == 0x3C:
                combiner = (words[0], words[1])
            elif opcode == 0x3B:
                environment = (
                    (words[1] >> 24) & 0xFF,
                    (words[1] >> 16) & 0xFF,
                    (words[1] >> 8) & 0xFF,
                    words[1] & 0xFF,
                )

            tris = triangle_count(opcode)
            if opcode in TRIANGLES:
                tile = (words[0] >> 16) & 7
                image = tmem_images.get(tile_tmem[tile])
                if image is not None and image[0] in target_addresses:
                    state = (
                        opcode,
                        tile,
                        image[0],
                        combiner,
                        other_mode_high,
                        other_mode_low,
                        environment,
                    )
                    road_state_counts[state] += tris
                    road_events.append(
                        {
                            "frame": int(record["frame"]),
                            "dlist_sequence": int(record["dlist_sequence"]),
                            "rdp_span": filename,
                            "command_index": command_index,
                            "triangle_index": triangle_index,
                            "triangles": tris,
                            "opcode": f"0x{opcode:02X}",
                            "tile": tile,
                            "texture_address": f"0x{image[0]:08X}",
                        }
                    )
                triangle_index += tris
            cursor += length
            command_index += 1

    def state_json(state: tuple[Any, ...], count: int) -> dict[str, Any]:
        opcode, tile, texture, comb, other_h, other_l, env = state
        mode = render_mode(other_l[1] if other_l is not None else 0)
        return {
            "triangles": count,
            "opcode": f"0x{opcode:02X}",
            "tile": tile,
            "texture_address": f"0x{texture:08X}",
            "combiner_word0": None if comb is None else f"0x{comb[0]:08X}",
            "combiner_word1": None if comb is None else f"0x{comb[1]:08X}",
            "other_mode_high": None if other_h is None else f"0x{other_h[1]:08X}",
            "other_mode_low": None if other_l is None else f"0x{other_l[1]:08X}",
            "environment_rgba": None if env is None else list(env),
            "render_mode": mode,
        }

    return {
        "source_xrtp_textures": source_textures,
        "target_addresses": [f"0x{value:08X}" for value in sorted(target_addresses)],
        "commands": command_index,
        "triangles_before_and_during_capture": triangle_index,
        "road_draw_events": len(road_events),
        "road_triangles": sum(int(event["triangles"]) for event in road_events),
        "first_road_event": road_events[0] if road_events else None,
        "last_road_event": road_events[-1] if road_events else None,
        "road_states": [
            state_json(state, count)
            for state, count in road_state_counts.most_common()
        ],
    }


def renderer_contract(renderer: str) -> dict[str, bool]:
    return {
        "generic_world_transparent_predicate_removed":
            "bool worldTransparent" not in renderer,
        "road_comment_mentions_opaque_z_source":
            "Dreamland's road is opaque and writes Z" in renderer,
        "water_requires_explicit_provenance":
            "f.DreamlandWater &&\n            coherentRasterDepth" in renderer,
        "only_water_or_glass_tests_transparent_depth":
            "(sourceOccludedTransparent || glassTransparent)" in renderer,
        "n64_route_uses_source_depth_compare":
            "f.N64RouteDepthCompare &&" in renderer and
            "sourceOpaqueDepthTest ||" in renderer,
        "n64_route_uses_single_pass_depth_compare_write":
            "Begin(f, 3, depthTest, depthWrite, sourceDepthCompareWrite)" in renderer and
            "if (_kDepthWrite && !_kSourceDepthCompareWrite)" in renderer,
        "terrain_route_trace_counts_depth_write":
            "route-depth-write=" in renderer,
        "terrain_route_trace_counts_depth_test":
            "route-depth-test=" in renderer,
        "terrain_route_trace_counts_depth_compare_write":
            "route-depth-compare-write=" in renderer,
    }


def parse_key_values(line: str) -> dict[str, str]:
    values: dict[str, str] = {}
    for match in re.finditer(r"([A-Za-z0-9_-]+)=([^ ]+)", line):
        values[match.group(1)] = match.group(2)
    return values


def port_trace_summary(
    summary_path: Path,
    stderr_path: Path,
    capture_path: Path,
    n64_framebuffer: Path | None,
    side_by_side: Path | None,
) -> tuple[dict[str, Any], list[str]]:
    failures: list[str] = []
    summary = json.loads(summary_path.read_text(encoding="utf-8-sig"))
    if not isinstance(summary, list) or not summary:
        raise ValueError(f"{summary_path} does not contain a run-result list")
    run = summary[0]
    if not isinstance(run, dict):
        raise ValueError(f"{summary_path} first entry is not an object")
    if not run.get("passed"):
        failures.append(f"port smoke did not pass: {run.get('reason')}")
    if run.get("arena") != "super_dreamland_64":
        failures.append(f"port smoke arena changed: {run.get('arena')}")
    if int(run.get("last_gameplay_tick", 0)) < 360:
        failures.append("port smoke did not reach gameplay tick 360")
    if not run.get("clean_match_exit"):
        failures.append("port smoke did not cleanly exit through the game")

    lines = stderr_path.read_text(encoding="utf-8", errors="replace").splitlines()
    route_intervals: list[dict[str, int]] = []
    color_summaries: list[dict[str, int]] = []
    route_packet_words: Counter[int] = Counter()
    route_vertex_samples: Counter[int] = Counter()
    deterministic_rng_seeds: list[int] = []
    for line in lines:
        if "[EnhancedRenderer]" in line:
            values = parse_key_values(line)
            terrain_route = int(values.get("terrain-route", "0"))
            if terrain_route <= 0:
                continue
            item = {
                "terrain_route": terrain_route,
                "route_opaque": int(values.get("route-opaque", "0")),
                "route_transparent": int(values.get("route-transparent", "0")),
                "route_depth_write": int(values.get("route-depth-write", "0")),
                "route_depth_test": int(values.get("route-depth-test", "0")),
                "route_depth_compare_write": int(
                    values.get("route-depth-compare-write", "0")
                ),
                "dreamland_water": int(values.get("dreamland-water", "0")),
                "water_transparent": int(values.get("water-transparent", "0")),
                "water_coherent": int(values.get("water-coherent", "0")),
            }
            route_intervals.append(item)
        elif "[V8N64RouteColorSummary]" in line:
            values = parse_key_values(line)
            color_summaries.append(
                {
                    "tick": int(values.get("tick", "0")),
                    "mapped": int(values.get("mapped", "0")),
                    "rejected": int(values.get("rejected", "0")),
                }
            )
        elif "[EnhancedPacketOwner]" in line and "terrain-route=1" in line:
            values = parse_key_values(line)
            route_packet_words[int(values.get("words", "0"))] += 1
        elif "[V8N64RouteVertexColor]" in line:
            values = parse_key_values(line)
            if "index" in values:
                route_vertex_samples[int(values["index"])] += 1
        elif "[V8DeterministicRng]" in line:
            values = parse_key_values(line)
            deterministic_rng_seeds.append(int(values.get("effective", "-1")))

    if not route_intervals:
        failures.append("port trace contains no terrain-route render intervals")
    for index, item in enumerate(route_intervals):
        if item["route_opaque"] != item["terrain_route"]:
            failures.append(f"port interval {index} did not render every route triangle opaque")
        if item["route_transparent"] != 0:
            failures.append(f"port interval {index} rendered route triangles as transparent")
        if item["route_depth_write"] != item["terrain_route"]:
            failures.append(f"port interval {index} did not depth-write every route triangle")
        if item["route_depth_test"] != item["terrain_route"]:
            failures.append(
                f"port interval {index} did not depth-test every N64 route triangle"
            )
        if item["route_depth_compare_write"] != item["terrain_route"]:
            failures.append(
                f"port interval {index} did not use single-pass depth compare/write "
                "for every N64 route triangle"
            )
    if not color_summaries:
        failures.append("port trace contains no N64 route-colour summary")
    if any(item["mapped"] <= 0 or item["rejected"] != 0 for item in color_summaries):
        failures.append("port route-colour mapping had zero mapped vertices or rejections")
    if not route_packet_words:
        failures.append("port trace contains no source-tagged terrain-route packets")
    if any(words != 12 for words in route_packet_words):
        failures.append(f"port terrain-route packets were not GT4 words: {sorted(route_packet_words)}")
    if not deterministic_rng_seeds or len(set(deterministic_rng_seeds)) != 1:
        failures.append("port trace lacks one stable deterministic game RNG seed")
    if not capture_path.exists():
        failures.append(f"port gameplay capture is missing: {capture_path}")

    capture_size: list[int] | None = None
    side_by_side_path: str | None = None
    if capture_path.exists():
        from PIL import Image, ImageDraw

        port_image = Image.open(capture_path).convert("RGB")
        capture_size = [port_image.width, port_image.height]
        if n64_framebuffer is not None and side_by_side is not None:
            n64_image = Image.open(n64_framebuffer).convert("RGB")
            target_h = max(n64_image.height, port_image.height)
            def scale_to_height(image: Image.Image) -> Image.Image:
                if image.height == target_h:
                    return image
                width = max(1, round(image.width * target_h / image.height))
                return image.resize((width, target_h), Image.Resampling.BILINEAR)
            left = scale_to_height(n64_image)
            right = scale_to_height(port_image)
            label_h = 20
            sheet = Image.new("RGB", (left.width + right.width, target_h + label_h), "black")
            sheet.paste(left, (0, label_h))
            sheet.paste(right, (left.width, label_h))
            draw = ImageDraw.Draw(sheet)
            draw.text((4, 3), "N64 reference frame 5460", fill="white")
            draw.text((left.width + 4, 3), "Port gameplay capture", fill="white")
            side_by_side.parent.mkdir(parents=True, exist_ok=True)
            sheet.save(side_by_side)
            side_by_side_path = str(side_by_side.resolve())

    return (
        {
            "summary": run,
            "route_intervals": route_intervals,
            "route_triangles": sum(item["terrain_route"] for item in route_intervals),
            "route_depth_write_triangles": sum(item["route_depth_write"] for item in route_intervals),
            "route_transparent_triangles": sum(item["route_transparent"] for item in route_intervals),
            "route_depth_test_triangles": sum(item["route_depth_test"] for item in route_intervals),
            "route_depth_compare_write_triangles": sum(
                item["route_depth_compare_write"] for item in route_intervals
            ),
            "route_packet_word_counts": dict(sorted(route_packet_words.items())),
            "route_color_summaries": color_summaries,
            "route_vertex_sample_indices": dict(sorted(route_vertex_samples.items())),
            "deterministic_rng_seeds": deterministic_rng_seeds,
            "gameplay_capture": str(capture_path.resolve()) if capture_path.exists() else str(capture_path),
            "gameplay_capture_size": capture_size,
            "side_by_side": side_by_side_path,
        },
        failures,
    )


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    capture = (
        root
        / "artifacts"
        / "n64_reference"
        / "dreamland-spawn-road-reference-20260814"
    )
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--capture", type=Path, default=capture)
    parser.add_argument(
        "--repeat-capture",
        type=Path,
        help=(
            "optional second hidden run; every synchronized framebuffer, "
            "RSP memory dump, and ordered RDP digest must match"
        ),
    )
    parser.add_argument(
        "--n64-arena",
        type=Path,
        default=root / "artifacts" / "n64_reference" / "static" / "DREAMLND_N64.EXP",
    )
    parser.add_argument(
        "--converted-arena",
        type=Path,
        default=root / "artifacts" / "n64_reference" / "static" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--rdp-texture-state",
        type=Path,
        default=capture / "rdp-texture-state.json",
    )
    parser.add_argument(
        "--rsp-route-vertices",
        type=Path,
        default=capture / "rsp-route-vertices-frame5460.json",
    )
    parser.add_argument(
        "--loader",
        type=Path,
        default=root / "analysis" / "dll" / "LOAD" / "decomp" / "80102bd4.c",
    )
    parser.add_argument(
        "--converter",
        type=Path,
        default=root / "tools" / "v8_n64_level.py",
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
        "--port-stability",
        type=Path,
        default=None,
        help="optional port-side runtime stability JSON to fold into the proof",
    )
    parser.add_argument(
        "--port-repeatability",
        type=Path,
        default=None,
        help="optional corrected-port repeatability JSON to fold into the proof",
    )
    parser.add_argument("--port-summary", type=Path)
    parser.add_argument("--port-stderr", type=Path)
    parser.add_argument("--port-capture", type=Path)
    parser.add_argument(
        "--n64-framebuffer",
        type=Path,
        default=capture / "n64_frame_5460.ppm",
    )
    parser.add_argument("--side-by-side", type=Path)
    parser.add_argument("--output", type=Path, default=capture / "spawn-road-proof.json")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    failures: list[str] = []
    capture = args.capture
    manifest = load_json(capture / "run_manifest.json")
    rdp = load_json(args.rdp_texture_state)
    rsp = load_json(args.rsp_route_vertices)

    n64_arena = args.n64_arena.read_bytes()
    converted_arena = args.converted_arena.read_bytes()
    n64_xrtp, n64_rseg = route_records(n64_arena)
    converted_xrtp, converted_rseg = route_records(converted_arena)
    if not manifest.get("hidden_desktop"):
        failures.append("reference run was not marked hidden-desktop")
    if "dreamland_gameplay_probe.csv" not in str(manifest.get("input_script", "")):
        failures.append("reference run did not use the Dreamland gameplay input script")
    if int(manifest.get("stop_frame", 0)) < 5900:
        failures.append("reference run stopped before the spawn-road window ended")
    if not manifest.get("dump_rdram"):
        failures.append("reference run did not dump RDRAM")
    if not manifest.get("dump_rdp_commands"):
        failures.append("reference run did not dump RDP commands")

    harness_bin = Path(__file__).resolve().parent / "bin"
    provenance_paths = {
        "headless_sha256": harness_bin / "v8_n64_headless.exe",
        "input_plugin_sha256": harness_bin / "v8_n64_input.dll",
        "gfx_proxy_sha256": harness_bin / "v8_n64_gfx_trace.dll",
        "audio_proxy_sha256": harness_bin / "v8_n64_audio_trace.dll",
        "audio_null_sha256": harness_bin / "v8_n64_audio_null.dll",
        "rsp_proxy_sha256": harness_bin / "v8_n64_rsp_trace.dll",
        "build_manifest_sha256": harness_bin / "build_manifest.json",
        "input_script_sha256": Path(str(manifest.get("input_script", ""))),
        "rom_sha256": Path(str(manifest.get("rom", ""))),
    }
    provenance_checks: dict[str, bool] = {
        "run_manifest_v2": manifest.get("schema") == "v8.n64-reference-run.v2",
        "deterministic_null_audio": manifest.get("audio_mode") == "null",
    }
    for field, path in provenance_paths.items():
        expected = str(manifest.get(field, "")).upper()
        matches = bool(expected) and path.is_file() and sha256_file(path) == expected
        provenance_checks[f"{field}_matches"] = matches
    for name, passed in provenance_checks.items():
        if not passed:
            failures.append(f"reference provenance failed: {name}")

    if len(n64_xrtp) != 2 or len(converted_xrtp) != 2:
        failures.append("expected two XRTP descriptors in source and converted arenas")
    if len(n64_rseg) != 35 or len(converted_rseg) != 35:
        failures.append("expected 35 RSEG records in source and converted arenas")
    n64_primary_flags = n64_xrtp[0]["flags"] if n64_xrtp else 0
    converted_primary_flags = converted_xrtp[0]["flags"] if converted_xrtp else 0
    if not (n64_primary_flags & 0x0100):
        failures.append("N64 source primary XRTP no longer carries bit 0x0100")
    if converted_primary_flags & 0x0100:
        failures.append("converted primary XRTP still carries PS1 semitrans bit 0x0100")

    source_route_states = [
        state for state in rdp.get("triangle_states", [])
        if isinstance(state, dict) and state.get("source_xrtp_index") == 0
    ]
    if len(source_route_states) != 1:
        failures.append(
            f"expected one runtime RDP state for source XRTP 0, found {len(source_route_states)}"
        )
    for state in source_route_states:
        mode = state.get("render_mode", {})
        if (
            not isinstance(mode, dict)
            or not mode.get("z_compare")
            or not mode.get("z_update")
            or mode.get("z_mode") != "opaque"
            or mode.get("force_blend")
        ):
            failures.append("runtime source XRTP 0 is not opaque Z-compare/Z-update")

    if rsp.get("status") != "PASS":
        failures.append("RSP route vertex audit did not pass")
    vertices = rsp.get("vertices", [])
    if not isinstance(vertices, list) or not vertices:
        failures.append("RSP route vertex audit produced no vertices")
        vertices = []
    if any(vertex.get("rgba", [0, 0, 0, 0])[3] != 255 for vertex in vertices):
        failures.append("RSP route vertices are not uniformly alpha 255")
    y_values = [
        int(vertex["position"][1])
        for vertex in vertices
        if isinstance(vertex, dict) and "position" in vertex
    ]
    if not y_values or min(y_values) == max(y_values):
        failures.append("runtime route vertices do not show terrain-conformed height variation")

    loader = args.loader.read_text(encoding="utf-8", errors="replace")
    if loader.count("Terrain_HeightAt") < 3:
        failures.append("recovered route builder lacks center/edge terrain-height sampling")
    converter = args.converter.read_text(encoding="utf-8", errors="replace")
    if "flags = be16(descriptor, 10) & ~0x0100" not in converter:
        failures.append("converter does not strip source-only XRTP bit 0x0100")
    renderer_checks = renderer_contract(
        args.renderer.read_text(encoding="utf-8", errors="replace")
    )
    for name, passed in renderer_checks.items():
        if not passed:
            failures.append(f"renderer contract failed: {name}")

    synchronization, synchronization_failures = capture_synchronization(
        capture,
        manifest.get("capture_frames"),
    )
    failures.extend(synchronization_failures)
    repeat_determinism: dict[str, Any] | None = None
    rsp_task_determinism: dict[str, Any] | None = None
    route_rdram_determinism: dict[str, Any] | None = None
    if args.repeat_capture is not None:
        repeat_manifest = load_json(args.repeat_capture / "run_manifest.json")
        repeat_synchronization, repeat_failures = capture_synchronization(
            args.repeat_capture,
            repeat_manifest.get("capture_frames"),
        )
        failures.extend(
            f"repeat run: {failure}" for failure in repeat_failures
        )
        repeat_determinism, comparison_failures = compare_synchronization(
            synchronization,
            repeat_synchronization,
        )
        failures.extend(comparison_failures)
        rsp_task_determinism, rsp_task_failures = compare_rsp_tasks(
            capture,
            args.repeat_capture,
        )
        failures.extend(rsp_task_failures)
        route_rdram_determinism, route_rdram_failures = compare_route_rdram(
            capture,
            args.repeat_capture,
            synchronization,
            repeat_synchronization,
            rsp,
        )
        failures.extend(route_rdram_failures)
    dmem_paths = sorted(capture.glob("n64_frame_*_dlist_*.dmem.bin"))
    framebuffer_paths = sorted(capture.glob("n64_frame_*.ppm"))
    rdp_span_count = len(list(capture.glob("n64_frame_*_rdp_*.bin")))
    frame5460_rdram = next(capture.glob("n64_frame_5460_dlist_*.rdram.bin"), None)
    if frame5460_rdram is None:
        failures.append("frame 5460 RDRAM dump is missing")
        submission = {}
    else:
        submission = scan_rdp_submission(
            capture, n64_arena, frame5460_rdram.read_bytes(), 0
        )
        if int(submission.get("road_triangles", 0)) <= 0:
            failures.append("RDP submission scan found no XRTP 0 road triangles")

    port_stability: dict[str, Any] | None = None
    if args.port_stability is not None:
        port_stability = load_json(args.port_stability)
        if port_stability.get("status") != "PASS":
            failures.append("port-side runtime stability proof did not pass")
    port_repeatability: dict[str, Any] | None = None
    if args.port_repeatability is not None:
        port_repeatability = load_json(args.port_repeatability)
        if port_repeatability.get("status") != "PASS":
            failures.append("corrected-port repeatability proof did not pass")
    port_trace: dict[str, Any] | None = None
    if args.port_summary or args.port_stderr or args.port_capture:
        if not (args.port_summary and args.port_stderr and args.port_capture):
            failures.append(
                "--port-summary, --port-stderr, and --port-capture must be provided together"
            )
        else:
            port_trace, port_failures = port_trace_summary(
                args.port_summary,
                args.port_stderr,
                args.port_capture,
                args.n64_framebuffer,
                args.side_by_side,
            )
            failures.extend(port_failures)

    def bounds(axis: int) -> list[int] | None:
        values = [
            int(vertex["position"][axis])
            for vertex in vertices
            if isinstance(vertex, dict) and "position" in vertex
        ]
        return [min(values), max(values)] if values else None

    report = {
        "schema": "v8.n64-dreamland-spawn-road-reference.v2",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "capture": str(capture.resolve()),
        "reference_run": {
            "hidden_desktop": manifest.get("hidden_desktop"),
            "stop_frame": manifest.get("stop_frame"),
            "capture_frames": manifest.get("capture_frames"),
            "dump_rdram": manifest.get("dump_rdram"),
            "dump_rdp_commands": manifest.get("dump_rdp_commands"),
            "framebuffer_captures": len(framebuffer_paths),
            "dmem_rdram_pairs": len(dmem_paths),
            "rdp_span_count": rdp_span_count,
            "synchronization": synchronization,
            "repeat_determinism": repeat_determinism,
            "rsp_task_determinism": rsp_task_determinism,
            "route_rdram_determinism": route_rdram_determinism,
            "provenance": provenance_checks,
        },
        "source_geometry": {
            "n64_xrtp": n64_xrtp,
            "converted_xrtp": converted_xrtp,
            "rseg_count": len(n64_rseg),
            "route_type_counts": dict(
                Counter(record["route_type"] for record in n64_rseg)
            ),
        },
        "runtime_geometry": {
            "rsp_frame": 5460,
            "triangles": rsp.get("rsp_triangles"),
            "unique_vertices": rsp.get("unique_vertex_addresses"),
            "bbox": {
                "x": bounds(0),
                "y": bounds(1),
                "z": bounds(2),
            },
            "gradient_indices_present": rsp.get("gradient_indices_present"),
            "route_states": rsp.get("route_states"),
        },
        "submission_order": submission,
        "rdp_depth_render_state": source_route_states,
        "runtime_code_alters_geometry": {
            "terrain_height_calls_in_builder": loader.count("Terrain_HeightAt"),
            "runtime_y_varies": bool(y_values and min(y_values) != max(y_values)),
            "evidence": (
                "RSEG/XRTP describe the route surface, while the recovered "
                "route builder terrain-conforms center and edge vertices before "
                "submitting the RSP display list."
            ),
        },
        "renderer_contract": renderer_checks,
        "port_stability": port_stability,
        "port_repeatability": port_repeatability,
        "port_trace": port_trace,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "status": report["status"],
        "failures": failures,
        "reference_run": report["reference_run"],
        "runtime_geometry": report["runtime_geometry"],
        "submission_order": {
            "road_triangles": submission.get("road_triangles"),
            "first_road_event": submission.get("first_road_event"),
            "last_road_event": submission.get("last_road_event"),
            "road_states": submission.get("road_states"),
        },
    }, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
