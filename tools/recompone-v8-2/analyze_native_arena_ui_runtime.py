#!/usr/bin/env python3
"""Reduce verbose selector/loading traces to a native arena UI contract."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re


TABLE_RE = re.compile(
    r"\[V82ArenaTable\].*resources=(?P<resources>\d+) "
    r"previews=(?P<previews>\d+) background-index=(?P<background>\d+) "
    r"background-offset=0x(?P<offset>[0-9A-F]+) "
    r"background-bytes=(?P<bytes>\d+)"
)
PREVIEW_RE = re.compile(
    r"\[V82ArenaTable\] slot=(?P<slot>\d+) id=(?P<id>\S+) "
    r"preview-index=(?P<preview>\d+) preview-offset=0x(?P<offset>[0-9A-F]+) "
    r"preview-bytes=(?P<bytes>\d+)"
)
MDEC_RE = re.compile(
    r"\[MDEC\] decode depth=(?P<depth>\d+).*mbs=(?P<mbs>\d+) "
    r"wordsOut=(?P<words>\d+).*hash=0x(?P<hash>[0-9A-F]+)"
)
MEDIA = {".ppm", ".png", ".jpg", ".jpeg", ".bmp", ".webp", ".gif", ".mp4"}
ERRORS = (
    "unhandled exception", "fatal error", "unmapped call:",
    "accessviolationexception", "outofmemoryexception",
)


def one_match(pattern: re.Pattern[str], text: str, label: str) -> re.Match[str]:
    matches = list(pattern.finditer(text))
    if len(matches) != 1:
        raise ValueError(f"expected one {label}, found {len(matches)}")
    return matches[0]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_dir", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    summary = json.loads((args.run_dir / "summary.json").read_text(encoding="utf-8"))
    runs = summary.get("runs", [])
    if len(runs) != 1 or not runs[0].get("passed"):
        raise ValueError("runtime result is not one passing run")
    run = runs[0]
    if run.get("actual_overlay") != "LEVELS_N64_DREAMLND":
        raise ValueError("runtime did not launch the registered Dreamland overlay")
    stdout = Path(run["stdout_log"]).read_text(encoding="utf-8", errors="replace")
    stderr = Path(run["stderr_log"]).read_text(encoding="utf-8", errors="replace")
    combined = stdout + "\n" + stderr
    lower = combined.lower()
    found_errors = [token for token in ERRORS if token in lower]
    if found_errors:
        raise ValueError("runtime log contains failure markers: " + ", ".join(found_errors))

    required = (
        "[V82Arena] validated 1 native imported arena",
        "[V82Arena] native locations retail=18 imported=1 total=19",
        "[V82Arena] location=n64.super_dreamland_64",
        "[V82Arena] launch stable-id=n64.super_dreamland_64 path=LEVELS\\N64\\DREAMLND.EXP",
    )
    missing = [line for line in required if line not in stdout]
    if missing:
        raise ValueError("runtime log is missing selector lifecycle entries")

    table = one_match(TABLE_RE, stdout, "selector resource-table trace")
    preview = one_match(PREVIEW_RE, stdout, "registered preview trace")
    if table.groupdict() != {
        "resources": "20", "previews": "19", "background": "19",
        "offset": table.group("offset"), "bytes": "29516",
    }:
        raise ValueError("selector resource-table contract is not 19 previews plus one background")
    if preview.group("slot") != "18" or preview.group("preview") != "18" or preview.group("bytes") != "16824":
        raise ValueError("Dreamland did not bind directly to native preview slot 18")

    decodes = [match for match in MDEC_RE.finditer(stdout) if match.group("mbs") == "120"]
    if len(decodes) != 1:
        raise ValueError(f"expected one 320x96 loading decode, found {len(decodes)}")
    decode = decodes[0]
    if decode.group("depth") != "3" or decode.group("words") != "15360":
        raise ValueError("loading decode does not match one 320x96 15-bit MDEC frame")
    overlay_loads = stdout.count("loaded relocated overlay: LEVELS_N64_DREAMLND")
    if overlay_loads != 1:
        raise ValueError(f"Dreamland overlay loaded {overlay_loads} times")

    media = [str(path) for path in args.run_dir.rglob("*") if path.is_file() and path.suffix.lower() in MEDIA]
    if media:
        raise ValueError("text-only run retained media: " + ", ".join(media))

    report = {
        "schema": 1,
        "status": "PASS",
        "selector": {
            "retailLocations": 18,
            "registeredLocations": 1,
            "selectedSlot": 18,
            "previewIndex": 18,
            "previewBytes": 16824,
            "resourceCount": 20,
            "previewCount": 19,
            "backgroundCount": 1,
            "backgroundIndex": 19,
            "backgroundBytes": 29516,
        },
        "loading": {
            "mdecFrames320x96": 1,
            "macroblocks": 120,
            "wordsOut": 15360,
            "decodeHash": decode.group("hash"),
            "overlayLoads": overlay_loads,
        },
        "gameplayFrames": run.get("last_frame"),
        "failureMarkers": [],
        "retainedMedia": [],
    }
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print("[NativeArenaUiRuntime] PASS")
    print(
        "[NativeArenaUiRuntime] selector slot=18 preview=18 "
        "resources=20 composition=19-previews+1-background"
    )
    print(
        f"[NativeArenaUiRuntime] loading mdec-320x96=1 macroblocks=120 "
        f"words=15360 hash=0x{decode.group('hash')} overlay-loads=1"
    )
    print(
        f"[NativeArenaUiRuntime] gameplay-frames={run.get('last_frame')} "
        "fatal-markers=0 retained-media=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
