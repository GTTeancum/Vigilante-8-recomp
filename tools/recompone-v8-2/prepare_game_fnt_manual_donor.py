#!/usr/bin/env python3
"""Register a manually redrawn GAME.FNT sheet to the original glyph grid."""

from __future__ import annotations

import argparse
import csv
import importlib.util
from pathlib import Path
import sys

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
DEFAULT_FNT = ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"
DEFAULT_OUTPUT = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" /
    "font_work" / "game_fnt_manual_donor"
)
PS_BUTTON_CODES = {0x80: "circle", 0x81: "square"}
ORIGINAL_FALLBACK_CODES = {
    0x60: "missing-backtick",
    0x7C: "missing-vertical-bar",
    **PS_BUTTON_CODES,
}
DONOR_TEXT_REPAIRS = {
    0x2E: ".",
    0x2F: "/",
    0x5C: "\\",
    0x5D: "]",
    0x74: "t",
    0x75: "u",
}
DEFAULT_REPAIR_FONT = Path("C:/Windows/Fonts/timesi.ttf")
DONOR_BINARY_THRESHOLD = 128
DONOR_COMPONENT_BOXES = {
    0x6C: (705, 11, 723, 54),
    0x6D: (723, 23, 772, 54),
    0x6E: (775, 23, 807, 54),
    0x6F: (812, 23, 842, 54),
    0x70: (839, 23, 878, 67),
}

# Measured on the normalized 1024x360 donor. Each row has one boundary per
# glyph plus its terminal edge, in the exact order stored by GAME.FNT.
MANUAL_ROW_BOUNDARIES = {
    0: [
        0, 16, 25, 76, 110, 163, 204, 215, 239, 262, 299, 338, 351,
        376, 378, 419, 450, 472, 507, 540, 575, 614, 651, 685, 720,
        754, 775, 795, 826, 856, 895, 928, 982, 1024,
    ],
    18: [
        0, 40, 86, 134, 188, 237, 291, 345, 370, 404, 455, 495,
        564, 624, 672, 713, 758, 799, 837, 879, 923, 966, 1024,
    ],
    36: [
        0, 49, 93, 135, 162, 239, 252, 272, 315, 323, 359, 397,
        431, 470, 507, 542, 580, 619, 638, 662, 701, 730, 774, 809,
        844, 879, 919, 949, 978, 995, 1024,
    ],
    54: [
        0, 42, 101, 143, 186, 220, 245, 246, 267, 300, 328, 343,
        428, 466, 539, 609, 685, 735, 776, 819, 864, 910, 953, 1024,
    ],
    72: [
        0, 49, 100, 131, 163, 195, 240, 284, 329, 373, 417, 460,
        491, 526, 559, 594, 627, 662, 695, 730, 766, 787, 813, 836,
        869, 903, 937, 971, 1004, 1024,
    ],
}


def load_fnt_tool():
    spec = importlib.util.spec_from_file_location("build_fnt_font_proof", FNT_TOOL)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {FNT_TOOL}")
    module = importlib.util.module_from_spec(spec)
    sys.modules["build_fnt_font_proof"] = module
    spec.loader.exec_module(module)
    return module


def artwork_alpha(
    image: Image.Image,
    black_point: int = 2,
    white_point: int = 255,
) -> Image.Image:
    rgb = image.convert("RGB")
    alpha = Image.new("L", rgb.size)
    source = rgb.load()
    target = alpha.load()
    for y in range(rgb.height):
        for x in range(rgb.width):
            value = max(source[x, y])
            target[x, y] = (
                0 if value <= black_point else
                255 if value >= white_point else
                round((value - black_point) * 255 /
                      (white_point - black_point))
            )
    white = Image.new("RGBA", rgb.size, (255, 255, 255, 0))
    white.putalpha(alpha)
    return white


def binary_artwork(image: Image.Image, threshold: int) -> Image.Image:
    rgb = image.convert("RGB")
    alpha = Image.new("L", rgb.size)
    alpha.putdata([
        255 if max(pixel) >= threshold else 0
        for pixel in rgb.getdata()
    ])
    white = Image.new("RGBA", rgb.size, (255, 255, 255, 0))
    white.putalpha(alpha)
    return white


def force_binary_alpha(image: Image.Image, threshold: int = 128) -> Image.Image:
    binary = image.convert("RGBA")
    binary.putalpha(binary.getchannel("A").point(
        lambda value: 255 if value >= threshold else 0))
    return binary


def connected_component_crop(
    image: Image.Image,
    expected_bbox: tuple[int, int, int, int],
) -> Image.Image:
    alpha = bytearray(image.getchannel("A").tobytes())
    seen = bytearray(len(alpha))
    components: list[tuple[tuple[int, int, int, int], list[int]]] = []
    for start, value in enumerate(alpha):
        if value == 0 or seen[start]:
            continue
        stack = [start]
        seen[start] = 1
        pixels: list[int] = []
        while stack:
            offset = stack.pop()
            pixels.append(offset)
            x = offset % image.width
            y = offset // image.width
            for adjacent_y in range(max(0, y - 1), min(image.height, y + 2)):
                for adjacent_x in range(max(0, x - 1), min(image.width, x + 2)):
                    adjacent = adjacent_y * image.width + adjacent_x
                    if alpha[adjacent] and not seen[adjacent]:
                        seen[adjacent] = 1
                        stack.append(adjacent)
        xs = [offset % image.width for offset in pixels]
        ys = [offset // image.width for offset in pixels]
        components.append(((min(xs), min(ys), max(xs) + 1, max(ys) + 1), pixels))
    if not components:
        raise ValueError("binary donor row contains no connected components")
    def distance(item) -> int:
        bbox, _pixels = item
        return sum(abs(actual - expected)
                   for actual, expected in zip(bbox, expected_bbox))
    bbox, pixels = min(components, key=distance)
    if distance((bbox, pixels)) != 0:
        raise ValueError(
            f"expected donor component {expected_bbox}, closest was {bbox}")
    out = Image.new("RGBA", (bbox[2] - bbox[0], bbox[3] - bbox[1]),
                    (255, 255, 255, 0))
    out_alpha = bytearray(out.width * out.height)
    for offset in pixels:
        x = offset % image.width
        y = offset // image.width
        out_alpha[(y - bbox[1]) * out.width + x - bbox[0]] = 255
    out.putalpha(Image.frombytes("L", out.size, bytes(out_alpha)))
    return out


def visible_bbox(image: Image.Image, threshold: int) -> tuple[int, int, int, int] | None:
    mask = image.getchannel("A").point(
        lambda value: 255 if value >= threshold else 0)
    return mask.getbbox()


def remove_small_components(
    image: Image.Image,
    threshold: int = 32,
    minimum_area: int = 25,
) -> tuple[Image.Image, int]:
    alpha = image.getchannel("A")
    pixels = bytearray(alpha.tobytes())
    seen = bytearray(len(pixels))
    removed = 0
    for start, value in enumerate(pixels):
        if value < threshold or seen[start]:
            continue
        stack = [start]
        seen[start] = 1
        component: list[int] = []
        while stack:
            offset = stack.pop()
            component.append(offset)
            x = offset % image.width
            y = offset // image.width
            for adjacent_y in range(max(0, y - 1), min(image.height, y + 2)):
                for adjacent_x in range(max(0, x - 1), min(image.width, x + 2)):
                    adjacent = adjacent_y * image.width + adjacent_x
                    if pixels[adjacent] >= threshold and not seen[adjacent]:
                        seen[adjacent] = 1
                        stack.append(adjacent)
        if len(component) >= minimum_area:
            continue
        removed += 1
        for offset in component:
            pixels[offset] = 0
    cleaned_alpha = Image.frombytes("L", image.size, bytes(pixels))
    cleaned = image.copy()
    cleaned.putalpha(cleaned_alpha)
    return cleaned, removed


def normalize_donor(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    binary = binary_artwork(image, DONOR_BINARY_THRESHOLD)
    if binary.size == size:
        return binary
    return binary.resize(size, Image.Resampling.NEAREST)


def row_boundaries(
    image: Image.Image,
    glyphs,
    atlas_width: int,
    search_radius: int,
    guide_font: ImageFont.FreeTypeFont | None,
) -> list[int]:
    alpha = image.getchannel("A")
    projection = [
        sum(1 for y in range(alpha.height) if alpha.getpixel((x, y)) >= 8)
        for x in range(alpha.width)
    ]
    ink_prefix = [0]
    for value in projection:
        ink_prefix.append(ink_prefix[-1] + value)

    chars = "".join(glyph.char for glyph in glyphs)
    if guide_font is not None and all(0x20 <= glyph.code <= 0x7E for glyph in glyphs):
        raw = [guide_font.getlength(chars[:index]) for index in range(len(chars) + 1)]
        total = max(1.0, raw[-1])
        expected = [round(value * image.width / total) for value in raw]
    else:
        expected = [round(glyph.x * image.width / atlas_width) for glyph in glyphs]
        expected.append(image.width)

    states: dict[int, tuple[float, list[int]]] = {0: (0.0, [0])}
    for index in range(1, len(glyphs)):
        target = expected[index]
        target_width = expected[index] - expected[index - 1]
        low = max(2, target - search_radius)
        high = min(image.width - 2, target + search_radius)
        next_states: dict[int, tuple[float, list[int]]] = {}
        for boundary in range(low, high + 1):
            edge_cost = (
                projection[boundary] * 100.0 +
                projection[max(0, boundary - 1)] * 20.0 +
                projection[min(image.width - 1, boundary + 1)] * 20.0
            )
            best: tuple[float, list[int]] | None = None
            for previous, (score, path) in states.items():
                if boundary - previous < 2:
                    continue
                if ink_prefix[boundary] == ink_prefix[previous]:
                    continue
                width_error = (boundary - previous) - target_width
                candidate = (
                    score + edge_cost + abs(boundary - target) * 2.0 +
                    width_error * width_error * 0.15,
                    path + [boundary],
                )
                if best is None or candidate[0] < best[0]:
                    best = candidate
            if best is not None:
                next_states[boundary] = best
        if not next_states:
            raise ValueError(f"cannot segment donor row at glyph index {index}")
        states = next_states

    final_best: tuple[float, list[int]] | None = None
    final_target_width = expected[-1] - expected[-2]
    for previous, (score, path) in states.items():
        if image.width - previous < 2:
            continue
        if ink_prefix[image.width] == ink_prefix[previous]:
            continue
        width_error = (image.width - previous) - final_target_width
        candidate = (score + width_error * width_error * 0.15, path + [image.width])
        if final_best is None or candidate[0] < final_best[0]:
            final_best = candidate
    if final_best is None:
        raise ValueError("cannot terminate donor-row segmentation")
    return final_best[1]


def fit_to_original_bounds(
    donor: Image.Image,
    original_cell: Image.Image,
) -> tuple[Image.Image, str, tuple[int, int, int, int] | None]:
    donor_bbox = visible_bbox(donor, 8)
    target_bbox = visible_bbox(original_cell, 128)
    out = Image.new("RGBA", original_cell.size, (0, 0, 0, 0))
    if donor_bbox is None or target_bbox is None:
        return out, "empty", donor_bbox
    content = donor.crop(donor_bbox)
    target_width = target_bbox[2] - target_bbox[0]
    target_height = target_bbox[3] - target_bbox[1]
    content = content.resize(
        (target_width, target_height),
        Image.Resampling.LANCZOS,
    )
    content = force_binary_alpha(content)
    out.alpha_composite(content, (target_bbox[0], target_bbox[1]))
    return out, "fit-original-bbox", donor_bbox


def render_missing_text_glyph(
    char: str,
    original_cell: Image.Image,
    font: ImageFont.FreeTypeFont,
) -> Image.Image:
    canvas = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    bbox = draw.textbbox((0, 0), char, font=font)
    draw.text(
        (16 - bbox[0], 16 - bbox[1]),
        char,
        font=font,
        fill=(255, 255, 255, 255),
    )
    rendered_bbox = visible_bbox(canvas, 1)
    target_bbox = visible_bbox(original_cell, 128)
    out = Image.new("RGBA", original_cell.size, (0, 0, 0, 0))
    if rendered_bbox is None or target_bbox is None:
        return out
    rendered = canvas.crop(rendered_bbox).resize(
        (target_bbox[2] - target_bbox[0], target_bbox[3] - target_bbox[1]),
        Image.Resampling.LANCZOS,
    )
    rendered = force_binary_alpha(rendered)
    out.alpha_composite(rendered, (target_bbox[0], target_bbox[1]))
    return out


def warp_measured_row(
    donor_row: Image.Image,
    donor_boundaries: list[int],
    glyphs,
    scale: int,
    target_width: int,
) -> Image.Image:
    """Warp one row continuously through every measured glyph boundary."""
    target_boundaries = [glyph.x * scale for glyph in glyphs]
    target_boundaries.append((glyphs[-1].x + glyphs[-1].width) * scale)
    mesh = []
    for source_x0, source_x1, target_x0, target_x1 in zip(
        donor_boundaries,
        donor_boundaries[1:],
        target_boundaries,
        target_boundaries[1:],
    ):
        if target_x1 <= target_x0:
            raise ValueError(
                f"non-increasing target interval {target_x0}..{target_x1}")
        mesh.append((
            (target_x0, 0, target_x1, donor_row.height),
            (
                source_x0, 0,
                source_x0, donor_row.height,
                source_x1, donor_row.height,
                source_x1, 0,
            ),
        ))
    return donor_row.transform(
        (target_width, donor_row.height),
        Image.Transform.MESH,
        mesh,
        resample=Image.Resampling.BICUBIC,
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--donor", type=Path, required=True)
    parser.add_argument("--fnt", type=Path, default=DEFAULT_FNT)
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--boundary-search", type=int, default=22)
    parser.add_argument("--guide-font", type=Path)
    parser.add_argument("--repair-font", type=Path, default=DEFAULT_REPAIR_FONT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    fnt = load_fnt_tool()
    sheet = fnt.decode_fnt(args.fnt)
    target_size = (sheet.atlas.width * args.scale, sheet.atlas.height * args.scale)
    supplied = Image.open(args.donor)
    normalized = normalize_donor(supplied, (supplied.width, target_size[1]))
    original = artwork_alpha(sheet.atlas).resize(
        target_size, Image.Resampling.NEAREST)
    aligned = Image.new("RGBA", target_size, (0, 0, 0, 0))
    guide_font = (
        ImageFont.truetype(str(args.guide_font), 64)
        if args.guide_font is not None else None
    )
    if not args.repair_font.is_file():
        raise FileNotFoundError(f"missing text-repair font: {args.repair_font}")
    repair_font = ImageFont.truetype(str(args.repair_font), 72)

    donor_cells: dict[int, Image.Image] = {}
    donor_bounds: dict[int, tuple[int, int]] = {}
    registration_rows: list[tuple[int, Image.Image, list, list[int]]] = []
    for row_y in sorted({glyph.y for glyph in sheet.glyphs}):
        glyphs = [glyph for glyph in sheet.glyphs if glyph.y == row_y]
        row = normalized.crop((
            0,
            row_y * args.scale,
            normalized.width,
            (row_y + sheet.max_height) * args.scale,
        ))
        boundaries = MANUAL_ROW_BOUNDARIES.get(row_y)
        if boundaries is None:
            boundaries = row_boundaries(
                row,
                glyphs,
                sheet.atlas.width,
                args.boundary_search,
                guide_font,
            )
        if len(boundaries) != len(glyphs) + 1:
            raise ValueError(
                f"row {row_y} has {len(glyphs)} glyphs but "
                f"{len(boundaries) - 1} measured intervals")
        registration_rows.append((row_y, row.copy(), glyphs, boundaries))
        for index, glyph in enumerate(glyphs):
            if glyph.code in DONOR_COMPONENT_BOXES:
                component_bbox = DONOR_COMPONENT_BOXES[glyph.code]
                donor_cells[glyph.code] = connected_component_crop(
                    row, component_bbox)
                donor_bounds[glyph.code] = (
                    component_bbox[0], component_bbox[2])
            else:
                donor_cells[glyph.code] = row.crop((
                    boundaries[index], 0, boundaries[index + 1], row.height))
                donor_bounds[glyph.code] = (
                    boundaries[index], boundaries[index + 1])

    rows: list[dict[str, object]] = []
    adjusted = 0
    empty = 0
    removed_components = 0
    for glyph in sheet.glyphs:
        box = (
            glyph.x * args.scale,
            glyph.y * args.scale,
            (glyph.x + glyph.width) * args.scale,
            (glyph.y + sheet.max_height) * args.scale,
        )
        donor_cell = donor_cells[glyph.code]
        before = visible_bbox(donor_cell, 8)
        source = "manual"
        if glyph.code in ORIGINAL_FALLBACK_CODES:
            cell = original.crop(box)
            action = "original-fallback"
            source = ORIGINAL_FALLBACK_CODES[glyph.code]
            aligned.paste((0, 0, 0, 0), box)
            aligned.alpha_composite(cell, (box[0], box[1]))
        elif glyph.code in DONOR_TEXT_REPAIRS:
            cell = render_missing_text_glyph(
                DONOR_TEXT_REPAIRS[glyph.code],
                original.crop(box),
                repair_font,
            )
            action = "times-italic-repair"
            source = f"missing-{DONOR_TEXT_REPAIRS[glyph.code]}-repair"
            aligned.paste((0, 0, 0, 0), box)
            aligned.alpha_composite(cell, (box[0], box[1]))
        else:
            donor_cell, pruned = remove_small_components(donor_cell)
            cell, action, before = fit_to_original_bounds(
                donor_cell, original.crop(box))
            if pruned:
                action += f"+pruned-{pruned}"
                removed_components += pruned
            aligned.alpha_composite(cell, (box[0], box[1]))
        after = visible_bbox(cell, 8)
        if action.startswith("fit-original-bbox"):
            adjusted += 1
        if after is None:
            empty += 1
        rows.append({
            "code": f"0x{glyph.code:02X}",
            "char": repr(glyph.char),
            "source": source,
            "cell_x": box[0],
            "cell_y": box[1],
            "cell_width": box[2] - box[0],
            "cell_height": box[3] - box[1],
            "advance_native": glyph.advance,
            "offset_native": glyph.offset,
            "bbox_before": "" if before is None else ",".join(map(str, before)),
            "bbox_after": "" if after is None else ",".join(map(str, after)),
            "donor_x0": donor_bounds[glyph.code][0],
            "donor_x1": donor_bounds[glyph.code][1],
            "action": action,
            "touch_left_after": int(after is not None and after[0] == 0),
            "touch_right_after": int(after is not None and after[2] == box[2] - box[0]),
            "touch_top_after": int(after is not None and after[1] == 0),
            "touch_bottom_after": int(after is not None and after[3] == box[3] - box[1]),
        })

    aligned = force_binary_alpha(aligned)

    args.output.mkdir(parents=True, exist_ok=True)
    binary_source = binary_artwork(supplied, DONOR_BINARY_THRESHOLD)
    binary_preview = Image.new("RGB", supplied.size, (0, 0, 0))
    binary_preview.paste(
        (255, 255, 255),
        mask=binary_source.getchannel("A"),
    )
    binary_preview.save(
        args.output / "game_fnt_manual_binary_source.png")
    normalized.save(args.output / "game_fnt_manual_normalized_4x.tga", format="TGA")
    aligned.save(args.output / "game_fnt_manual_aligned_4x.tga", format="TGA")
    for row_y, row, glyphs, boundaries in registration_rows:
        registration = Image.new("RGBA", (row.width, row.height + 34), (12, 12, 12, 255))
        registration.alpha_composite(row, (0, 34))
        registration_draw = ImageDraw.Draw(registration)
        for index, glyph in enumerate(glyphs):
            x = boundaries[index]
            registration_draw.line((x, 0, x, registration.height), fill=(255, 48, 48, 255))
            registration_draw.text(
                (x + 2, 2 + (index & 1) * 13),
                f"{glyph.code:02X}",
                fill=(255, 220, 80, 255),
            )
        registration_draw.line(
            (boundaries[-1] - 1, 0, boundaries[-1] - 1, registration.height),
            fill=(255, 48, 48, 255),
        )
        registration.convert("RGB").save(
            args.output / f"game_fnt_registration_row_{row_y // sheet.max_height}.png")
    with (args.output / "game_fnt_manual_glyph_audit.csv").open(
        "w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    columns = 8
    panel_w = 150
    panel_h = sheet.max_height * args.scale + 34
    proof = Image.new(
        "RGBA",
        (columns * panel_w, ((len(rows) + columns - 1) // columns) * panel_h),
        (12, 12, 12, 255),
    )
    draw = ImageDraw.Draw(proof)
    glyph_by_code = {glyph.code: glyph for glyph in sheet.glyphs}
    for index, row in enumerate(rows):
        glyph = glyph_by_code[int(str(row["code"]), 16)]
        px = (index % columns) * panel_w
        py = (index // columns) * panel_h
        box = (
            glyph.x * args.scale,
            glyph.y * args.scale,
            (glyph.x + glyph.width) * args.scale,
            (glyph.y + sheet.max_height) * args.scale,
        )
        cell = aligned.crop(box)
        proof.alpha_composite(cell, (px + 4, py + 24))
        draw.rectangle(
            (px + 3, py + 23, px + 4 + cell.width, py + 24 + cell.height),
            outline=(80, 80, 80, 255),
        )
        draw.text(
            (px + 4, py + 5),
            f"{row['code']} {row['char']} {row['action']}",
            fill=(230, 230, 230, 255),
        )
    proof.convert("RGB").save(args.output / "game_fnt_manual_glyph_proof.png")

    edge_left = sum(int(row["touch_left_after"]) for row in rows)
    edge_right = sum(int(row["touch_right_after"]) for row in rows)
    edge_top = sum(int(row["touch_top_after"]) for row in rows)
    edge_bottom = sum(int(row["touch_bottom_after"]) for row in rows)
    report = [
        f"donor={args.donor}",
        f"supplied_size={supplied.size[0]}x{supplied.size[1]}",
        f"normalized_working_size={normalized.width}x{normalized.height}",
        f"aligned_size={target_size[0]}x{target_size[1]}",
        f"glyph_records={len(rows)}",
        f"per_glyph_bbox_fits={adjusted}",
        f"empty_glyphs={empty}",
        f"removed_noise_components={removed_components}",
        f"edge_left_after={edge_left}",
        f"edge_right_after={edge_right}",
        f"edge_top_after={edge_top}",
        f"edge_bottom_after={edge_bottom}",
        f"donor_binary_threshold={DONOR_BINARY_THRESHOLD}",
        "text_repairs=0x2E/period,0x2F/slash,0x5C/backslash,"
        f"0x5D/right-bracket,0x74/t,0x75/u via {args.repair_font}",
        "original_fallbacks=0x60/backtick,0x7C/vertical-bar,0x80/circle,0x81/square",
    ]
    (args.output / "game_fnt_manual_report.txt").write_text(
        "\n".join(report) + "\n", encoding="utf-8")
    print("\n".join(report))


if __name__ == "__main__":
    main()
