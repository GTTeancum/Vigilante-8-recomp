#!/usr/bin/env python3
"""Smoke every character through Quest mode and report which ones hang.

TO-DO item: "Game froze when starting Chassey Blue's quest. Smoke quest mode
with each character."

The soak harness enters Arcade. Quest sits one entry below it on the mode menu,
so the only change needed is a DOWN press before that menu is confirmed; the
character itself is installed through RECOMPONE_V82_PLAYER_TYPE rather than by
counting carousel presses, exactly as the arcade path does.

A hang shows up as a run that never reports gameplay, or one whose process has
to be killed. Both are reported per character so the failure can be narrowed to
specific slots rather than "quest mode is broken".
"""
from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "tools/recompone-v8-2"
GAMEPLAY = re.compile(r"gameplay reached")
RESULT = re.compile(r"^\[soak\] (PASS|FAIL).*reason=(.*)$", re.M)


def quest_script(slot: int, character_slot: int = 0) -> str:
    """Arcade script with the mode menu moved down one entry to Quest."""
    import run_reference_soak as soak
    script = soak._original_build_input_script(slot, character_slot)
    return script.replace("1200+2=SELECT", "1200+3=DOWN", 1)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--characters", type=int, default=18)
    ap.add_argument("--frames", type=int, default=600)
    ap.add_argument("--map", type=int, default=0)
    ap.add_argument("--timeout", type=int, default=420)
    ap.add_argument("--output", type=Path,
                    default=REPO / "artifacts/quest-smoke")
    args = ap.parse_args()

    results = []
    for character in range(args.characters):
        out = args.output / f"c{character:02d}"
        out.mkdir(parents=True, exist_ok=True)
        env = dict(os.environ)
        env["RECOMPONE_V82_SOAK_POWERUPS"] = "0"
        env["RECOMPONE_V82_QUEST_SMOKE"] = "1"
        baseline = REPO / "artifacts/terrain-fix/interface.ini.baseline"
        if baseline.is_file():
            import shutil
            shutil.copy2(baseline, REPO / "V8_2_LOOSE/interface.ini")
        try:
            proc = subprocess.run(
                [sys.executable, str(TOOLS / "run_quest_smoke_child.py"),
                 "--map", str(args.map), "--character", str(character),
                 "--frames", str(args.frames), "--output", str(out)],
                env=env, cwd=str(REPO), capture_output=True, text=True,
                timeout=args.timeout)
            stdout = proc.stdout or ""
            hung = False
        except subprocess.TimeoutExpired as expired:
            stdout = (expired.stdout or b"").decode("utf-8", "replace") \
                if isinstance(expired.stdout, bytes) else (expired.stdout or "")
            hung = True
        reached = bool(GAMEPLAY.search(stdout))
        found = RESULT.search(stdout)
        verdict = ("HUNG" if hung
                   else "no gameplay" if not reached
                   else found.group(1) if found else "unknown")
        reason = found.group(2) if found and not hung else ""
        results.append((character, verdict, reason))
        print(f"  character {character:2d}: {verdict} {reason}", flush=True)

    print("\nsummary")
    bad = [r for r in results if r[1] not in ("PASS",)]
    for character, verdict, reason in results:
        mark = "" if verdict == "PASS" else "   <=="
        print(f"  {character:2d}  {verdict}{mark}")
    print(f"\n{len(results) - len(bad)}/{len(results)} characters completed "
          f"a quest run")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
