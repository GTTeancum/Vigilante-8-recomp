#!/usr/bin/env python3
"""Map live loading-screen GAME.FNT crops to donor glyphs by tip text."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import importlib.util
import json
from pathlib import Path
import re
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[2]
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
SUPPLEMENT_TOOL = (
    ROOT / "tools" / "recompone-v8-2" / "apply_loading_game_font_supplement.py"
)
DEFAULT_FNT = ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"
DEFAULT_LOAD = ROOT / "V8_2_LOOSE" / "SHELL" / "LOAD.DLL"
DEFAULT_MANIFEST = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "manifest.json"
)
MOD_FONT_WORK = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "font_work"
)


@dataclass(frozen=True)
class GlyphUse:
    key: str
    width: int
    height: int
    uv: str
    tpage: str
    clut: str
    x: float
    y: float


@dataclass(frozen=True)
class TextMatch:
    use: GlyphUse
    char: str


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def extract_tips(path: Path) -> list[str]:
    payload = path.read_bytes()
    tips: list[str] = []
    for match in re.finditer(rb"TIP:\x01PPP  ([^\x00]+)", payload):
        text = match.group(1).decode("latin1")
        tips.append(text.replace("\\", "-"))
    if not tips:
        raise ValueError(f"no loading tips found in {path}")
    return tips


def collect_uses(trace_dirs: list[Path]) -> dict[Path, list[GlyphUse]]:
    pattern = re.compile(
        r"V82LoadingUiResolve.*key=([0-9a-f]{16}) hit=[01] "
        r"screen=([0-9.]+),([0-9.]+)-[^ ]+ "
        r"size=([0-9]+)x([0-9]+) uv=([^ ]+) "
        r"tpage=(0x0(?:08|A5)) clut=(0x780[0C])"
    )
    by_log: dict[Path, list[GlyphUse]] = {}
    for trace_dir in trace_dirs:
        for log in sorted(trace_dir.glob("*.stderr.log")):
            uses: list[GlyphUse] = []
            for line in log.read_text(encoding="utf-8", errors="ignore").splitlines():
                match = pattern.search(line)
                if match is None:
                    continue
                key, x_text, y_text, width_text, height_text, uv, tpage, clut = (
                    match.groups()
                )
                width = int(width_text)
                height = int(height_text)
                if height != 18:
                    continue
                uses.append(
                    GlyphUse(
                        key,
                        width,
                        height,
                        uv,
                        tpage,
                        clut,
                        float(x_text),
                        float(y_text),
                    )
                )
            if uses:
                by_log[log] = uses
    return by_log


def nonspace(text: str) -> str:
    return "".join(char for char in text if char != " ")


def row_uses(uses: list[GlyphUse]) -> list[list[GlyphUse]]:
    rows: dict[float, list[GlyphUse]] = {}
    for use in uses:
        if use.tpage != "0x008" or use.clut != "0x780C":
            continue
        rows.setdefault(use.y, []).append(use)
    result: list[list[GlyphUse]] = []
    for _y, row in sorted(rows.items()):
        visible = [
            use for use in sorted(row, key=lambda item: item.x)
            if use.key != "0000000000000000" and use.width > 0
        ]
        if visible:
            result.append(visible)
    return result


def row_splits(text: str, counts: list[int]) -> list[list[str]]:
    if len(counts) == 1:
        return [[text]] if len(nonspace(text)) == counts[0] else []
    spaces = [index for index, char in enumerate(text) if char == " "]
    results: list[list[str]] = []

    def walk(start: int, row_index: int, rows: list[str]) -> None:
        if row_index == len(counts) - 1:
            tail = text[start:].strip()
            if len(nonspace(tail)) == counts[row_index]:
                results.append(rows + [tail])
            return
        for split in spaces:
            if split < start:
                continue
            segment = text[start:split].strip()
            if len(nonspace(segment)) == counts[row_index]:
                walk(split + 1, row_index + 1, rows + [segment])

    walk(0, 0, [])
    return results


def glyph_widths(sheet) -> dict[str, int]:
    return {glyph.char: glyph.width for glyph in sheet.glyphs}


def best_text_rows(sheet, tips: list[str], rows: list[list[GlyphUse]]) -> list[str] | None:
    counts = [len(row) for row in rows]
    widths = glyph_widths(sheet)
    best: tuple[int, list[str]] | None = None
    for tip in tips:
        text = "TIP:  " + tip
        if len(nonspace(text)) != sum(counts):
            continue
        for split in row_splits(text, counts):
            score = 0
            for row_text, row_uses_list in zip(split, rows):
                chars = nonspace(row_text)
                for char, use in zip(chars, row_uses_list):
                    score += abs(widths.get(char, use.width) - use.width)
            if best is None or score < best[0]:
                best = (score, split)
    return None if best is None else best[1]


def runtime_hash(image: Image.Image) -> int:
    value = 14695981039346656037
    width, height = image.size
    for byte in (width & 0xFF, width >> 8, height & 0xFF, height >> 8):
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    for byte in image.tobytes():
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def load_runtime_crop(dump_dirs: list[Path], use: GlyphUse) -> Image.Image | None:
    name = f"{use.key}_{use.width}x{use.height}.rgba"
    for dump_dir in dump_dirs:
        path = dump_dir / name
        if not path.exists():
            continue
        image = Image.frombytes("RGBA", (use.width, use.height), path.read_bytes())
        if f"{runtime_hash(image):016x}" != use.key:
            raise ValueError(f"{path} hashes to the wrong key")
        return image
    return None


def crop_mask(image: Image.Image) -> bytes:
    rgba = image.convert("RGBA")
    pixels = rgba.tobytes()
    return bytes(
        1 if (pixels[index] or pixels[index + 1] or pixels[index + 2] or pixels[index + 3]) else 0
        for index in range(0, len(pixels), 4)
    )


def mask_diff(left: bytes, right: bytes) -> float:
    if len(left) != len(right):
        return 1.0
    return sum(a != b for a, b in zip(left, right)) / max(1, len(left))


def source_crop_for_char(
    fnt_module,
    sheet,
    char: str,
    use: GlyphUse,
    dump_dirs: list[Path],
    radius_x: int,
    radius_y: int,
) -> tuple[int, int, float]:
    glyph = next((glyph for glyph in sheet.glyphs if glyph.char == char), None)
    if glyph is None:
        raise ValueError(f"GAME.FNT has no glyph for {char!r}")
    runtime = load_runtime_crop(dump_dirs, use)
    atlas = fnt_module.opaque_crop(sheet.atlas)
    if runtime is None:
        x = glyph.x + max(0, (glyph.width - use.width) // 2)
        return x, glyph.y, 1.0
    runtime_mask = crop_mask(runtime)
    best: tuple[int, int, float] | None = None
    for dy in range(-radius_y, radius_y + 1):
        for dx in range(-radius_x, radius_x + 1):
            x = glyph.x + dx
            y = glyph.y + dy
            if x < 0 or y < 0:
                continue
            if x + use.width > atlas.width or y + use.height > atlas.height:
                continue
            crop = atlas.crop((x, y, x + use.width, y + use.height))
            score = mask_diff(runtime_mask, crop_mask(crop))
            if best is None or score < best[2]:
                best = (x, y, score)
    if best is None:
        raise ValueError(f"cannot place {char!r} for {use.key}")
    return best


def render_live_cell(
    char: str,
    use: GlyphUse,
    font: ImageFont.FreeTypeFont,
    scale: int,
    width_factor: float,
    hard_alpha: bool,
    x_offset: int,
    y_offset: int,
) -> Image.Image:
    target = Image.new("RGBA", (use.width * scale, use.height * scale), (0, 0, 0, 0))
    temp = Image.new(
        "RGBA",
        (max(64, use.width * scale * 6), max(64, use.height * scale * 6)),
        (0, 0, 0, 0),
    )
    draw = ImageDraw.Draw(temp)
    bbox = draw.textbbox((0, 0), char, font=font)
    draw.text(
        (-bbox[0] + scale, -bbox[1] + scale),
        char,
        font=font,
        fill=(255, 255, 255, 255),
    )
    rendered_box = temp.getbbox()
    if rendered_box is None:
        return target
    rendered = temp.crop(rendered_box)
    if width_factor != 1.0:
        rendered = rendered.resize(
            (max(1, int(round(rendered.width * width_factor))), rendered.height),
            Image.Resampling.LANCZOS,
        )
    if hard_alpha:
        alpha = rendered.getchannel("A").point(lambda value: 255 if value >= 24 else 0)
    else:
        alpha = rendered.getchannel("A").point(
            lambda value: 0 if value < 24 else 255 if value > 232 else value)
    white = Image.new("RGBA", rendered.size, (255, 255, 255, 0))
    white.putalpha(alpha)
    target.alpha_composite(white, (x_offset * scale, y_offset * scale))
    return target


def build_textmap_atlas(
    fnt_module,
    sheet,
    matches: dict[str, tuple[GlyphUse, str]],
    dump_dirs: list[Path],
    font_path: Path,
    point_size: int,
    scale: int,
    width_factor: float,
    hard_alpha: bool,
    cell_mode: str,
    x_offset: int,
    y_offset: int,
) -> tuple[str, Image.Image, list[dict[str, object]], dict[str, list[str]]]:
    source = fnt_module.render_fitted_ttf_atlas(
        sheet,
        font_path,
        point_size,
        scale,
        width_factor,
        shadow_alpha=0,
    )
    pixels = bytearray(source.convert("RGBA").tobytes())
    for index in range(0, len(pixels), 4):
        alpha = pixels[index + 3]
        if alpha < 24:
            pixels[index:index + 4] = b"\x00\x00\x00\x00"
        else:
            if hard_alpha:
                alpha = 255
            elif alpha > 232:
                alpha = 255
            pixels[index:index + 4] = bytes((255, 255, 255, alpha))
    source = Image.frombytes("RGBA", source.size, bytes(pixels))
    padding = max(2, scale // 2)
    max_width = 1936
    x = y = row_h = padding
    glyphs = {glyph.char: glyph for glyph in sheet.glyphs}
    placements: list[tuple[GlyphUse, str, Image.Image, int, int, int, int]] = []
    for key, (use, char) in sorted(matches.items(), key=lambda item: (item[1][0].height, item[1][0].width, item[0])):
        glyph = glyphs.get(char)
        if glyph is None:
            continue
        if cell_mode == "runtime":
            font = ImageFont.truetype(str(font_path), point_size * scale)
            cell = render_live_cell(
                char,
                use,
                font,
                scale,
                width_factor,
                hard_alpha,
                x_offset,
                y_offset,
            )
        else:
            cell = source.crop((
                glyph.x * scale,
                glyph.y * scale,
                (glyph.x + glyph.width) * scale,
                (glyph.y + sheet.max_height) * scale,
            ))
        if x + cell.width + padding > max_width:
            x = padding
            y += row_h + padding
            row_h = padding
        placements.append((use, char, cell, x, y, glyph.x, glyph.y))
        x += cell.width + padding
        row_h = max(row_h, cell.height)

    atlas = Image.new("RGBA", (max_width, y + row_h + padding), (0, 0, 0, 0))
    image_name = f"game_fnt_loading_textmap_{font_path.stem.lower()}_{scale}x.dds"
    entries: list[dict[str, object]] = []
    sources: dict[str, list[str]] = {}
    for use, char, cell, px, py, sx, sy in placements:
        atlas.alpha_composite(cell, (px, py))
        entries.append({
            "key": use.key,
            "image": f"images/ui/{image_name}",
            "x": px,
            "y": py,
            "width": cell.width,
            "height": cell.height,
        })
        sources[use.key] = [
            f"GAME.FNT live loading glyph textmap char={char!r} "
            f"uv={use.uv} size={use.width}x{use.height} "
            f"tpage={use.tpage} clut={use.clut} "
            f"source={sx},{sy} font={font_path.name} "
            f"point={point_size} width_factor={width_factor} "
            f"hard_alpha={1 if hard_alpha else 0} "
            f"cell_mode={cell_mode} x_offset={x_offset} y_offset={y_offset}"
        ]
    return image_name, atlas, entries, sources


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--trace-dir", type=Path, action="append", required=True)
    parser.add_argument("--dump-dir", type=Path, action="append", default=[])
    parser.add_argument("--fnt", type=Path, default=DEFAULT_FNT)
    parser.add_argument("--load-dll", type=Path, default=DEFAULT_LOAD)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--font", type=Path, required=True)
    parser.add_argument("--point-size", type=int, required=True)
    parser.add_argument("--width-factor", type=float, default=0.92)
    parser.add_argument("--hard-alpha", action="store_true")
    parser.add_argument("--cell-mode", choices=("runtime", "source"), default="runtime")
    parser.add_argument("--x-offset", type=int, default=0)
    parser.add_argument("--y-offset", type=int, default=-1)
    parser.add_argument("--scale", type=int, default=8)
    parser.add_argument("--radius-x", type=int, default=6)
    parser.add_argument("--radius-y", type=int, default=3)
    parser.add_argument("--proof-out", type=Path, default=MOD_FONT_WORK)
    args = parser.parse_args()

    fnt = load_module(FNT_TOOL, "build_fnt_font_proof")
    supplement = load_module(SUPPLEMENT_TOOL, "apply_loading_game_font_supplement")
    sheet = fnt.decode_fnt(args.fnt)
    tips = extract_tips(args.load_dll)
    uses_by_log = collect_uses(args.trace_dir)
    if not uses_by_log:
        raise SystemExit("no loading GAME.FNT trace lines found")

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    supplement.strip_existing(manifest)
    entries: list[dict[str, object]] = []
    sources: dict[str, list[str]] = {}
    pending: dict[str, tuple[GlyphUse, str]] = {}
    conflicts: list[str] = []
    unresolved: list[str] = []

    existing = {str(entry.get("key", "")) for entry in manifest.get("entries", [])}
    key_chars: dict[str, str] = {}
    for log, uses in uses_by_log.items():
        rows = row_uses(uses)
        if not rows:
            continue
        text_rows = best_text_rows(sheet, tips, rows)
        if text_rows is None:
            unresolved.append(str(log))
            continue
        for row_text, row in zip(text_rows, rows):
            for char, use in zip(nonspace(row_text), row):
                previous = key_chars.get(use.key)
                if previous is not None and previous != char:
                    conflicts.append(f"{use.key}: {previous!r} vs {char!r} in {log}")
                    continue
                key_chars[use.key] = char
                if use.key in existing:
                    continue
                pending[use.key] = (use, char)

    if conflicts:
        raise SystemExit("conflicting live glyph text map:\n" + "\n".join(conflicts[:20]))

    image_name, atlas, entries, sources = build_textmap_atlas(
        fnt,
        sheet,
        pending,
        args.dump_dir,
        args.font,
        args.point_size,
        args.scale,
        args.width_factor,
        args.hard_alpha,
        args.cell_mode,
        args.x_offset,
        args.y_offset,
    )
    atlas_path = args.manifest.parent / "images" / "ui" / image_name
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(atlas_path)

    manifest["entries"].extend(entries)
    manifest.setdefault("sources", {}).update(sources)
    suffix = f"; GAME.FNT live loading glyph textmap {args.scale}x font atlas"
    if suffix not in str(manifest.get("generator", "")):
        manifest["generator"] = str(manifest.get("generator", "")) + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    args.proof_out.mkdir(parents=True, exist_ok=True)
    report = [
        f"trace_dirs={';'.join(str(path) for path in args.trace_dir)}",
        f"dump_dirs={';'.join(str(path) for path in args.dump_dir)}",
        f"font_atlas={atlas_path}",
        f"logs_read={len(uses_by_log)}",
        f"entries_added={len(entries)}",
        f"existing_keys={len(existing)}",
        f"unresolved_logs={len(unresolved)}",
    ]
    report.extend(f"unresolved={path}" for path in unresolved)
    (args.proof_out / "game_fnt_loading_textmap_report.txt").write_text(
        "\n".join(report) + "\n",
        encoding="utf-8",
    )
    print("\n".join(report))


if __name__ == "__main__":
    main()
