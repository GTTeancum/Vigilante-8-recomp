#!/usr/bin/env python3
"""Compare original-V8 and retail-V8:2 object-bank hierarchy links."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import sys


REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

from vigilante8_vehicle_tools import iff, xobf


ARENAS = (
    "AIRGRAVE",
    "CANYNLND",
    "CASNOCTY",
    "HOOVRDAM",
    "OILFIELD",
    "SANDFACT",
    "SCRTBASE",
    "SKIRESRT",
    "VALLYFRM",
    "WILDWEST",
)


def banks(path: Path, dialect: str) -> list[tuple[xobf.Slot, ...]]:
    result = []
    for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
        chunks = {child.tag: child.payload for child in form.children}
        result.append(xobf.Model(chunks[b"BIN "], dialect=dialect).slots())
    return result


def record(arena: str, bank_index: int, source, target) -> dict[str, object]:
    count = min(len(source), len(target))
    exact = []
    changed = []
    for index in range(count):
        left = (source[index].next_sibling, source[index].first_child)
        right = (target[index].next_sibling, target[index].first_child)
        (exact if left == right else changed).append(index)
    def signature(slot):
        return (
            slot.x, slot.y, slot.z,
            slot.rot_y, slot.rot_x, slot.rot_z,
            slot.obstacle_index,
        )

    source_counts = Counter(signature(slot) for slot in source)
    target_counts = Counter(signature(slot) for slot in target)
    target_by_signature = {
        signature(slot): index
        for index, slot in enumerate(target)
        if target_counts[signature(slot)] == 1
    }
    mapping = {
        index: target_by_signature[signature(slot)]
        for index, slot in enumerate(source)
        if source_counts[signature(slot)] == 1
        and signature(slot) in target_by_signature
    }
    semantic_links = []
    for source_index, target_index in mapping.items():
        source_slot = source[source_index]
        target_slot = target[target_index]
        source_next = mapping.get(source_slot.next_sibling, 0xFFFF)
        source_child = mapping.get(source_slot.first_child, 0xFFFF)
        if (
            source_slot.next_sibling != 0xFFFF
            and source_slot.next_sibling not in mapping
        ) or (
            source_slot.first_child != 0xFFFF
            and source_slot.first_child not in mapping
        ):
            continue
        semantic_links.append({
            "source_slot": source_index,
            "target_slot": target_index,
            "source_mapped": [source_next, source_child],
            "target": [target_slot.next_sibling, target_slot.first_child],
        })
    semantic_changed = [
        item for item in semantic_links
        if item["source_mapped"] != item["target"]
    ]
    return {
        "arena": arena,
        "bank": bank_index,
        "source_slots": len(source),
        "target_slots": len(target),
        "shared_slots": count,
        "exact_links": len(exact),
        "changed_links": len(changed),
        "changed_examples": [
            {
                "slot": index,
                "source": [source[index].next_sibling, source[index].first_child],
                "target": [target[index].next_sibling, target[index].first_child],
                "source_key": source[index].render_key & 0xFFFF,
                "target_key": target[index].render_key & 0xFFFF,
            }
            for index in changed[:64]
        ],
        "unique_semantic_matches": len(mapping),
        "comparable_semantic_links": len(semantic_links),
        "changed_semantic_links": len(semantic_changed),
        "changed_semantic_examples": semantic_changed[:64],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts" / "v8-v82-hierarchy-link-audit.json",
    )
    args = parser.parse_args()
    records = []
    for arena in ARENAS:
        source_path = (
            REPO / "artifacts" / "v8-retail-extract-20260820" /
            "TERRAIN" / f"{arena}.EXP"
        )
        target_path = REPO / "V8_2_LOOSE" / "LEVELS" / "V8" / f"{arena}.EXP"
        source_banks = banks(source_path, "V8")
        target_banks = banks(target_path, "V8_2")
        for index in range(min(len(source_banks), len(target_banks))):
            records.append(record(
                arena, index, source_banks[index], target_banks[index]
            ))
    report = {"schema": "v8-v82-hierarchy-links-v1", "banks": records}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    for item in records:
        print(
            f"{item['arena']} bank={item['bank']} "
            f"slots={item['source_slots']}/{item['target_slots']} "
            f"index-changed={item['changed_links']} "
            f"semantic={item['changed_semantic_links']}/"
            f"{item['comparable_semantic_links']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
