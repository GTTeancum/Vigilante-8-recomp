#!/usr/bin/env python3
"""Deterministic non-interactive acceptance for V8:2 wrapper seams."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-probe-acceptance"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose", type=Path, default=DEFAULT_LOOSE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    exe = args.exe.resolve()
    loose = args.loose.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    portrait_output = output / "portraits"
    portrait_output.mkdir(exist_ok=True)
    cases = (
        (
            "graphics",
            ["--probe-graphics-config"],
            "[GraphicsConfig] preset=Enhanced",
        ),
        (
            "voices",
            ["--probe-result-voice-proxies", str(loose)],
            "[ResultVoiceProxy] vehicles=12",
        ),
        (
            "package",
            ["--probe-vehicle-package", str(loose)],
            "[VehiclePackage] vehicles=12 banks=36 portraits=12",
        ),
        (
            "selector",
            ["--probe-selector-lifecycle", str(loose)],
            "[SelectorLifecycle] vehicles=12 banks=36 portraits=12",
        ),
        (
            "lifetime",
            ["--probe-defeat-quit-lifetime", str(loose)],
            "[DefeatQuitLifetime] vehicles=12 banks=36 portraits=12",
        ),
        (
            "portraits",
            [
                "--probe-selector-portraits",
                str(loose),
                str(portrait_output),
            ],
            "[SelectorPortraitProbe] vehicles=12 banks=36 portraits=12 "
            "returned=12/12",
        ),
    )
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_MUTE": "1",
            "RECOMPONE_WINDOW_VISIBLE": "0",
            "RECOMPONE_SUPPRESS_RUMBLE": "1",
        }
    )
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    results: list[dict[str, object]] = []
    for name, command, marker in cases:
        completed = subprocess.run(
            [str(exe), *command],
            cwd=exe.parent,
            env=env,
            capture_output=True,
            creationflags=flags,
            check=False,
        )
        stdout = completed.stdout.decode("utf-8", errors="replace")
        stderr = completed.stderr.decode("utf-8", errors="replace")
        (output / f"{name}.stdout.log").write_text(
            stdout, encoding="utf-8")
        (output / f"{name}.stderr.log").write_text(
            stderr, encoding="utf-8")
        combined = stdout + "\n" + stderr
        graphics_contract = (
            "preset=Enhanced",
            "hle=enhanced",
            "projection=perspective",
            "dithering=False",
            "smoothing=True",
            "aa=FXAA",
            "msaa=2",
            "anisotropy=4",
            "mipmaps=True",
            "widescreen=True",
            "world-texture-class=512",
            "ui-texture-class=1024",
        )
        passed = (
            completed.returncode == 0
            and marker in combined
            and (
                name != "graphics"
                or all(token in combined for token in graphics_contract)
            )
            and "[Fatal]" not in combined
            and "FAILED:" not in combined
        )
        results.append(
            {
                "case": name,
                "exit_code": completed.returncode,
                "marker": marker,
                "graphics_contract":
                    list(graphics_contract) if name == "graphics" else None,
                "passed": passed,
            }
        )
        print(
            f"[ProbeAcceptance] case={name} pass={passed} "
            f"exit={completed.returncode}",
            flush=True,
        )

    report = {
        "schema": 1,
        "passed": all(bool(item["passed"]) for item in results),
        "cases_passed": sum(bool(item["passed"]) for item in results),
        "cases_total": len(results),
        "executable": str(exe),
        "executable_sha256":
            hashlib.sha256(exe.read_bytes()).hexdigest().upper(),
        "results": results,
    }
    (output / "acceptance.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
