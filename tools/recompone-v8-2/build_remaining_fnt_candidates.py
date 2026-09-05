#!/usr/bin/env python3
"""Build review-only 4x candidates for every V8:2 FNT without an HD sidecar."""

from __future__ import annotations

from dataclasses import dataclass
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import unicodedata

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
OUTPUT = ROOT / "mods" / "v82_hd_font_candidates"
SCALE = 4


@dataclass(frozen=True)
class Candidate:
    id: str
    label: str
    font: Path
    point_size: int
    width_factor: float
    shadow_alpha: int


@dataclass(frozen=True)
class FontSpec:
    id: str
    source: Path
    role: str
    candidates: tuple[Candidate, ...]


SPECS = (
    FontSpec(
        id="hud",
        source=LOOSE / "SHARED" / "HUD.FNT",
        role="compact in-match digits and HUD symbols",
        candidates=(
            Candidate(
                "agency_fb_bold",
                "Agency FB Bold",
                Path("C:/Windows/Fonts/AGENCYB.TTF"),
                18,
                0.94,
                0,
            ),
            Candidate(
                "arial_narrow_bold",
                "Arial Narrow Bold",
                Path("C:/Windows/Fonts/ARIALNB.TTF"),
                18,
                0.94,
                0,
            ),
            Candidate(
                "ocr_a_extended",
                "OCR A Extended",
                Path("C:/Windows/Fonts/OCRAEXT.TTF"),
                18,
                0.90,
                0,
            ),
        ),
    ),
    FontSpec(
        id="kong",
        source=LOOSE / "SHARED" / "KONG.FNT",
        role="heavy block diagnostic and exception-screen text",
        candidates=(
            Candidate(
                "rockwell_extra_bold",
                "Rockwell Extra Bold",
                Path("C:/Windows/Fonts/ROCKEB.TTF"),
                20,
                0.96,
                150,
            ),
            Candidate(
                "rockwell_condensed_bold",
                "Rockwell Condensed Bold",
                Path("C:/Windows/Fonts/ROCCB___.TTF"),
                20,
                0.98,
                150,
            ),
            Candidate(
                "cooper_black",
                "Cooper Black",
                Path("C:/Windows/Fonts/COOPBL.TTF"),
                20,
                0.94,
                150,
            ),
        ),
    ),
)


def load_fnt_tool():
    spec = importlib.util.spec_from_file_location("v82_fnt_candidate_decoder", FNT_TOOL)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {FNT_TOOL}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def proof_image(fnt, sheet, candidate: Image.Image, label: str) -> Image.Image:
    original = fnt.black_preview(sheet.atlas, SCALE)
    replacement = fnt.black_preview(candidate, 1)
    label_height = 30
    gap = 12
    width = max(original.width, replacement.width)
    height = label_height * 2 + original.height + replacement.height + gap
    proof = Image.new("RGB", (width, height), (10, 10, 10))
    draw = ImageDraw.Draw(proof)
    draw.text((8, 8), f"Original {sheet.path.name} at 4x nearest", fill=(235, 235, 235))
    proof.paste(original, (0, label_height))
    second_y = label_height + original.height + gap
    draw.text((8, second_y + 8), label, fill=(235, 235, 235))
    proof.paste(replacement, (0, second_y + label_height))
    return proof


def main() -> int:
    fnt = load_fnt_tool()
    # Candidate fonts must cover every printable record in the source file.
    # The older proof helper intentionally retained uncommon punctuation from
    # the retail atlas; that would leave visibly low-definition symbols inside
    # an otherwise-HD whole-file replacement, so broaden its donor predicate
    # for this candidate-only build.
    fnt.donor_text_character = lambda char: (
        ord(char) >= 0x20 and unicodedata.category(char)[0] != "C"
    )
    OUTPUT.mkdir(parents=True, exist_ok=True)
    index: dict[str, object] = {
        "purpose": "Review-only 4x whole-file FNT replacement candidates",
        "deployed": False,
        "scale": SCALE,
        "fonts": [],
    }
    for spec in SPECS:
        if not spec.source.is_file():
            raise FileNotFoundError(spec.source)
        sheet = fnt.decode_fnt(spec.source)
        font_dir = OUTPUT / spec.id
        font_dir.mkdir(parents=True, exist_ok=True)
        source = fnt.opaque_crop(sheet.atlas).convert("RGBA")
        source_path = font_dir / "original_atlas.png"
        source.save(source_path)
        font_record: dict[str, object] = {
            "id": spec.id,
            "source": spec.source.relative_to(LOOSE).as_posix(),
            "role": spec.role,
            "sourceWidth": sheet.atlas.width,
            "sourceHeight": sheet.atlas.height,
            "original": source_path.relative_to(ROOT).as_posix(),
            "candidates": [],
        }
        for choice in spec.candidates:
            if not choice.font.is_file():
                raise FileNotFoundError(choice.font)
            candidate_dir = font_dir / choice.id
            candidate_dir.mkdir(parents=True, exist_ok=True)
            atlas = fnt.render_fitted_ttf_atlas(
                sheet,
                choice.font,
                choice.point_size,
                SCALE,
                width_factor=choice.width_factor,
                shadow_alpha=choice.shadow_alpha,
            )
            png_path = candidate_dir / "atlas_4x.png"
            dds_path = candidate_dir / "atlas_4x.dds"
            proof_path = candidate_dir / "proof.png"
            atlas.save(png_path)
            atlas.save(dds_path)
            proof_image(
                fnt,
                sheet,
                atlas,
                f"Candidate: {choice.label} (whole-file 4x atlas)",
            ).save(proof_path)
            font_record["candidates"].append({
                "id": choice.id,
                "label": choice.label,
                "font": choice.font.as_posix(),
                "pointSize": choice.point_size,
                "widthFactor": choice.width_factor,
                "shadowAlpha": choice.shadow_alpha,
                "png": png_path.relative_to(ROOT).as_posix(),
                "dds": dds_path.relative_to(ROOT).as_posix(),
                "proof": proof_path.relative_to(ROOT).as_posix(),
                "pngSha256": sha256(png_path),
                "ddsSha256": sha256(dds_path),
            })
        index["fonts"].append(font_record)
    (OUTPUT / "index.json").write_text(
        json.dumps(index, indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"fonts={len(SPECS)} candidates={sum(len(spec.candidates) for spec in SPECS)}")
    print(f"output={OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
