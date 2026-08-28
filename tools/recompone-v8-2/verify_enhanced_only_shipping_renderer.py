#!/usr/bin/env python3
"""Prove that ordinary shipping configuration cannot select Original."""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = (
    REPO / "artifacts" / "v82-enhanced-only-shipping-renderer"
)
RUNTIME = (
    REPO / "tools" / "recompone-reference" / "RecompOne.Runtime"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def game_process_ids() -> list[int]:
    if os.name != "nt":
        return []
    completed = subprocess.run(
        [
            "tasklist", "/FI", "IMAGENAME eq Vigilante82PC.exe",
            "/FO", "CSV", "/NH",
        ],
        capture_output=True,
        text=True,
        check=False,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
    )
    result: list[int] = []
    for row in csv.reader(completed.stdout.splitlines()):
        if len(row) >= 2 and row[0].lower() == "vigilante82pc.exe":
            try:
                result.append(int(row[1]))
            except ValueError:
                pass
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--timeout", type=float, default=120.0)
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    output = args.output.resolve()
    if not exe.is_file():
        parser.error(f"executable not found: {exe}")
    if not (loose / "SYSTEM.CNF").is_file():
        parser.error(f"loose root lacks SYSTEM.CNF: {loose}")
    if game_process_ids():
        parser.error("refusing to start while Vigilante82PC is running")

    output.mkdir(parents=True, exist_ok=True)
    runtime_log = output / "runtime.log"
    stdout_log = output / "stdout.log"
    stderr_log = output / "stderr.log"
    report_path = output / "acceptance.json"
    for stale in (runtime_log, stdout_log, stderr_log, report_path):
        stale.unlink(missing_ok=True)

    display_source = (
        RUNTIME / "Host" / "Window" / "Settings" / "Sections" /
        "DisplaySettingsSection.cs"
    ).read_text(encoding="utf-8")
    host_source = (
        RUNTIME / "Host" / "Window" / "HostWindow.cs"
    ).read_text(encoding="utf-8")
    view_source = (
        RUNTIME / "Config" / "ViewConfig.cs"
    ).read_text(encoding="utf-8")

    with tempfile.TemporaryDirectory(
        prefix="enhanced-only-", dir=output
    ) as temporary:
        isolated = Path(temporary)
        isolated_exe = isolated / "Vigilante82PC.exe"
        fixture = isolated / "input.txt"
        shutil.copy2(exe, isolated_exe)
        # Simulate both a stale user INI and an attempted ordinary automation
        # override. Neither carries the explicit developer-oracle gate.
        (isolated / "interface.ini").write_text(
            "[RecompOne]\nGraphicsPreset=Original\n",
            encoding="utf-8",
        )
        fixture.write_text("0+1=START\n", encoding="utf-8")

        env = os.environ.copy()
        env.update({
            "RECOMPONE_INPUT_FILE": str(fixture),
            "RECOMPONE_DISABLE_LIVE_INPUT": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_MUTE": "1",
            "SDL_AUDIODRIVER": "dummy",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
            "RECOMPONE_UNTHROTTLED": "1",
            "RECOMPONE_GRAPHICS_PRESET": "Original",
            "RECOMPONE_SCRIPT_EXIT_AFTER_POLLS": "240",
            "RECOMPONE_DISABLE_SCRIPT_STAGE_CAPTURES": "1",
            "RECOMPONE_TRACE_ENHANCED_RENDERER": "1",
            "RECOMPONE_DISPLAY_PROBE_IMAGES": "0",
            "RECOMPONE_LOG_PATH": str(runtime_log),
            "RECOMPONE_MOD_DIR": str(loose / "mods"),
            "RECOMPONE_CAPTURE_DIR": str(output),
        })
        for key in (
            "RECOMPONE_GPU_HLE",
            "RECOMPONE_ORIGINAL_RENDERER_ORACLE",
            "RECOMPONE_CAPTURE_SCRIPTED_STAGE",
            "RECOMPONE_PRESENTATION_CAPTURE",
        ):
            env.pop(key, None)

        with stdout_log.open("wb") as stdout, stderr_log.open("wb") as stderr:
            completed = subprocess.run(
                [str(isolated_exe), "--loose", str(loose)],
                cwd=isolated,
                env=env,
                stdout=stdout,
                stderr=stderr,
                timeout=args.timeout,
                check=False,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
            )

    text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in (runtime_log, stdout_log, stderr_log)
        if path.is_file()
    )
    checks = {
        "clean_exit": completed.returncode == 0,
        "settings_menu_excludes_original": (
            'GraphicsPresets = ["Enhanced", "Custom"]' in display_source and
            'GraphicsPresets = ["Original"' not in display_source
        ),
        "software_path_requires_explicit_oracle_gate": (
            "OriginalRendererOracleEnabled" in host_source and
            "RECOMPONE_ORIGINAL_RENDERER_ORACLE" in view_source
        ),
        "stale_original_ini_migrated_to_enhanced": (
            "[Host] graphics preset=Enhanced" in text and
            "graphics preset=Original" not in text
        ),
        "attempted_original_override_stayed_enhanced": (
            "[EnhancedRenderer] frames=" in text and
            "renderer=PS1 software" not in text and
            "hle=False" not in text
        ),
        "no_second_game_process": not game_process_ids(),
        "no_images": not any(output.glob("*.ppm")),
    }
    passed = all(checks.values())
    report = {
        "schema": 1,
        "passed": passed,
        "candidate": str(exe),
        "candidate_sha256": sha256(exe),
        "attempted_configuration": {
            "interface_ini": "GraphicsPreset=Original",
            "environment": "RECOMPONE_GRAPHICS_PRESET=Original",
            "oracle_gate": "unset",
        },
        "observed_renderer": "Enhanced",
        "checks": checks,
        "logs": {
            "runtime": str(runtime_log),
            "stdout": str(stdout_log),
            "stderr": str(stderr_log),
        },
    }
    report_path.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    state = "PASS" if passed else "FAIL"
    failed = ",".join(name for name, ok in checks.items() if not ok)
    print(
        f"[EnhancedOnlyRenderer] {state} sha256={report['candidate_sha256']}" +
        (f" failed={failed}" if failed else "")
    )
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
