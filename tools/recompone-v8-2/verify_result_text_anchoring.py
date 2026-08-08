#!/usr/bin/env python3
"""Verify that widescreen HUD anchoring cannot split the result sentence.

The stock result renderer emits ``PLAYER 1 destroyed!`` as 19 adjacent
textured rectangles.  The former broad "left third" rule moved only the first
two glyphs by the widescreen margin, which produced the reported
``PL    AYER`` gap.  This verifier consumes rectangles traced from the native
software packet stream and applies the Enhanced anchoring classifier to the
exact source geometry.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import defaultdict
from pathlib import Path


RECT_RE = re.compile(
    r"^\[SoftwareRect\] tick=(?P<tick>\d+) .* "
    r"xy=(?P<x>-?\d+),(?P<y>-?\d+) "
    r"wh=(?P<w>\d+)x(?P<h>\d+) "
    r"uv=(?P<u>\d+),(?P<v>\d+) "
    r"tex=(?P<tex>[01]) "
)

# Width and atlas origin for the retail 19-rectangle sentence.  Zero-width
# space packets are intentionally retained because their authored advances
# are part of the spacing contract.
RESULT_SIGNATURE = (
    (9, 156, 18),
    (9, 106, 18),
    (11, 228, 0),
    (10, 12, 36),
    (11, 32, 18),
    (9, 176, 18),
    (0, 0, 0),
    (5, 104, 0),
    (0, 0, 0),
    (9, 100, 36),
    (7, 110, 36),
    (6, 222, 36),
    (5, 228, 36),
    (7, 214, 36),
    (7, 188, 36),
    (9, 34, 54),
    (7, 110, 36),
    (9, 100, 36),
    (4, 0, 0),
)


def parse(path: Path) -> list[dict[str, int]]:
    result: list[dict[str, int]] = []
    payload = path.read_bytes()
    encoding = "utf-16" if payload.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8"
    for line in payload.decode(encoding, errors="replace").splitlines():
        match = RECT_RE.match(line)
        if not match:
            continue
        item = {name: int(value) for name, value in match.groupdict().items()}
        result.append(item)
    return result


def find_result(rectangles: list[dict[str, int]]) -> list[dict[str, int]]:
    groups: dict[tuple[int, int], list[dict[str, int]]] = defaultdict(list)
    for rect in rectangles:
        # The retail renderer alternates between native Y=0 and Y=240 draw
        # buffers.  Normalize both to display-local geometry.
        local_y = rect["y"] % 240
        groups[(rect["tick"], local_y)].append(rect)
    signature_length = len(RESULT_SIGNATURE)
    for (_tick, local_y), group in sorted(groups.items()):
        if local_y != 104:
            continue
        group.sort(key=lambda item: item["x"])
        for start in range(0, len(group) - signature_length + 1):
            candidate = group[start : start + signature_length]
            signature = tuple(
                (rect["w"], rect["u"], rect["v"]) for rect in candidate
            )
            if signature == RESULT_SIGNATURE:
                return candidate
    raise ValueError("native PLAYER 1 destroyed! rectangle sequence not found")


def classify(
    rect: dict[str, int],
    *,
    target_width: int,
    target_height: int,
    margin: int,
    legacy: bool,
) -> int:
    local_top = rect["y"] % target_height
    local_center = rect["x"] + rect["w"] * 0.5
    top_gameplay_hud = local_top < target_height * 0.42
    top_hud = top_gameplay_hud
    location_caption = (
        local_top >= target_height * 0.52
        and local_top < target_height * 0.9
    )
    small_text = (
        rect["tex"] == 1
        and not top_gameplay_hud
        and rect["w"] <= 32
        and rect["h"] <= 32
    )
    lower_left_hud = (
        (legacy or not small_text)
        and local_center < target_width / 3.0
    ) or location_caption
    return -margin if top_hud or lower_left_hud else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    trace = args.trace.resolve()
    output = (
        args.output.resolve()
        if args.output
        else trace.with_name("result-text-anchoring.json")
    )
    rectangles = find_result(parse(trace))
    target_width, target_height = 320, 240
    wide_width = (target_width * 4 + 2) // 3
    margin = (wide_width - target_width + 1) // 2
    current = [
        classify(
            rect,
            target_width=target_width,
            target_height=target_height,
            margin=margin,
            legacy=False,
        )
        for rect in rectangles
    ]
    legacy = [
        classify(
            rect,
            target_width=target_width,
            target_height=target_height,
            margin=margin,
            legacy=True,
        )
        for rect in rectangles
    ]
    source_x = [rect["x"] for rect in rectangles]
    current_x = [x + anchor for x, anchor in zip(source_x, current)]
    checks = {
        "native_sentence_found": len(rectangles) == len(RESULT_SIGNATURE),
        "all_result_glyphs_bypass_hud_anchor": set(current) == {0},
        "native_inter_glyph_advances_preserved": [
            current_x[index + 1] - current_x[index]
            for index in range(len(current_x) - 1)
        ] == [
            source_x[index + 1] - source_x[index]
            for index in range(len(source_x) - 1)
        ],
        "negative_control_reproduces_reported_split":
            legacy[:2] == [-margin, -margin]
            and set(legacy[2:]) == {0},
    }
    report = {
        "schema": 1,
        "passed": all(checks.values()),
        "checks": checks,
        "native_target": {
            "width": target_width,
            "height": target_height,
            "wide_width": wide_width,
            "margin": margin,
        },
        "sentence": "PLAYER 1 destroyed!",
        "source_x": source_x,
        "current_anchors": current,
        "current_draw_x": current_x,
        "legacy_anchors": legacy,
        "legacy_draw_x": [
            x + anchor for x, anchor in zip(source_x, legacy)
        ],
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[ResultTextAnchoring] "
        f"{'PASS' if report['passed'] else 'FAIL'} "
        f"glyphs={len(rectangles)} margin={margin}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
