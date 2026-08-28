#!/usr/bin/env python3
"""Compatibility entry point for the universal file-provenance FNT pack."""

from __future__ import annotations

import runpy
from pathlib import Path


def main() -> None:
    tool = Path(__file__).with_name("build_file_font_replacements.py")
    runpy.run_path(str(tool), run_name="__main__")


if __name__ == "__main__":
    main()
