#!/usr/bin/env python3
"""Compare two V8 RecompOne JSONL state traces at field granularity."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any, Iterator


def load_trace(path: Path) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    text = path.read_text(encoding="utf-8")
    raw_lines = text.splitlines(keepends=True)
    if raw_lines and not raw_lines[-1].endswith(("\n", "\r")):
        # A running reference may be midway through its next buffered record.
        raw_lines.pop()
    lines = []
    for line_number, line in enumerate(raw_lines, 1):
        if not line.strip():
            continue
        try:
            lines.append(json.loads(line))
        except json.JSONDecodeError as error:
            raise ValueError(f"{path}:{line_number}: {error.msg}") from error
    if not lines or lines[0].get("record") != "schema":
        raise ValueError(f"{path}: missing state-trace schema record")
    schema = lines[0]
    states = lines[1:]
    if any(item.get("record") != "state" for item in states):
        raise ValueError(f"{path}: contains a non-state record after its schema")
    return schema, states


def walk_differences(
    expected: Any, actual: Any, path: str = ""
) -> Iterator[tuple[str, Any, Any]]:
    if type(expected) is not type(actual):
        yield path or "$", expected, actual
        return
    if isinstance(expected, dict):
        for key in sorted(expected.keys() | actual.keys()):
            child = f"{path}.{key}" if path else key
            if key not in expected:
                yield child, "<missing>", actual[key]
            elif key not in actual:
                yield child, expected[key], "<missing>"
            else:
                yield from walk_differences(expected[key], actual[key], child)
        return
    if isinstance(expected, list):
        if len(expected) != len(actual):
            yield f"{path}.length", len(expected), len(actual)
        for index, (left, right) in enumerate(zip(expected, actual)):
            yield from walk_differences(left, right, f"{path}[{index}]")
        return
    if expected != actual:
        yield path or "$", expected, actual


def is_ignored(field: str, ignored: set[str]) -> bool:
    return any(field == prefix or field.startswith(f"{prefix}.") for prefix in ignored)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Report the first divergent tick and fields in two V8 state traces."
    )
    parser.add_argument("expected", type=Path)
    parser.add_argument("actual", type=Path)
    parser.add_argument(
        "--ignore",
        action="append",
        default=[],
        metavar="FIELD",
        help="ignore a field path (repeatable, for example --ignore vsync)",
    )
    parser.add_argument("--max-diffs", type=int, default=16)
    parser.add_argument(
        "--allow-prefix",
        action="store_true",
        help="accept identical common records when one trace ends first",
    )
    args = parser.parse_args()

    try:
        expected_schema, expected_states = load_trace(args.expected)
        actual_schema, actual_states = load_trace(args.actual)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(error, file=sys.stderr)
        return 2

    if expected_schema.get("schema") != actual_schema.get("schema"):
        print(
            "schema mismatch: "
            f"{expected_schema.get('schema')} != {actual_schema.get('schema')}",
            file=sys.stderr,
        )
        return 2

    ignored = set(args.ignore)
    for index, (expected, actual) in enumerate(zip(expected_states, actual_states)):
        differences = [
            difference
            for difference in walk_differences(expected, actual)
            if not is_ignored(difference[0], ignored)
        ]
        if not differences:
            continue
        expected_tick = expected.get("tick", f"record {index + 1}")
        actual_tick = actual.get("tick", f"record {index + 1}")
        print(
            f"first divergence at record {index + 1}: "
            f"expected tick {expected_tick}, actual tick {actual_tick}"
        )
        for field, left, right in differences[: max(1, args.max_diffs)]:
            print(f"  {field}: expected={left!r} actual={right!r}")
        if len(differences) > args.max_diffs:
            print(f"  ... {len(differences) - args.max_diffs} more field differences")
        return 1

    if len(expected_states) != len(actual_states) and not args.allow_prefix:
        print(
            "trace length mismatch: "
            f"expected {len(expected_states)} states, actual {len(actual_states)}"
        )
        return 1

    compared = min(len(expected_states), len(actual_states))
    suffix = " (common prefix)" if len(expected_states) != len(actual_states) else ""
    print(f"state traces match for {compared} ticks{suffix}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
