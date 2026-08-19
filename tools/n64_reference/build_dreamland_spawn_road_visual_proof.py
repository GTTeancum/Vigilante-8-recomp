#!/usr/bin/env python3
"""Build a proof-driven N64/port spawn-road comparison board."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps


BACKGROUND = (15, 19, 22)
PANEL = (25, 31, 35)
TEXT = (236, 241, 239)
MUTED = (165, 178, 176)
SOURCE = (255, 190, 85)
PORT = (100, 222, 164)


def font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    names = (
        ("DejaVuSans-Bold.ttf", "arialbd.ttf")
        if bold
        else ("DejaVuSans.ttf", "arial.ttf")
    )
    for name in names:
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def parse_crop(value: str) -> tuple[int, int, int, int]:
    parts = value.split(",")
    if len(parts) != 4:
        raise argparse.ArgumentTypeError("crop must be x0,y0,x1,y1")
    try:
        crop = tuple(int(part) for part in parts)
    except ValueError as error:
        raise argparse.ArgumentTypeError("crop values must be integers") from error
    x0, y0, x1, y1 = crop
    if x1 <= x0 or y1 <= y0:
        raise argparse.ArgumentTypeError("crop must have positive dimensions")
    return crop


def fitted(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    return ImageOps.contain(image.convert("RGB"), size, Image.Resampling.LANCZOS)


def paste_centered(
    board: Image.Image,
    image: Image.Image,
    bounds: tuple[int, int, int, int],
) -> tuple[int, int]:
    x0, y0, x1, y1 = bounds
    scaled = fitted(image, (x1 - x0, y1 - y0))
    x = x0 + (x1 - x0 - scaled.width) // 2
    y = y0 + (y1 - y0 - scaled.height) // 2
    board.paste(scaled, (x, y))
    return x, y


def outlined_frame(
    image: Image.Image,
    crop: tuple[int, int, int, int],
    color: tuple[int, int, int],
) -> Image.Image:
    result = image.convert("RGB").copy()
    draw = ImageDraw.Draw(result)
    width = max(2, round(result.width / 240))
    draw.rectangle(crop, outline=color, width=width)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--proof", type=Path, required=True)
    parser.add_argument("--n64-frame", type=Path, required=True)
    parser.add_argument("--port-frame", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--n64-crop",
        type=parse_crop,
        default=(210, 250, 600, 445),
    )
    parser.add_argument(
        "--port-crop",
        type=parse_crop,
        default=(1050, 640, 1824, 1026),
    )
    args = parser.parse_args()

    proof = json.loads(args.proof.read_text(encoding="utf-8"))
    if proof.get("status") != "PASS":
        raise SystemExit("combined spawn-road proof is not PASS")
    repeat = proof.get("port_repeatability")
    if not isinstance(repeat, dict) or repeat.get("status") != "PASS":
        raise SystemExit("combined proof has no passing port repeatability gate")

    n64 = Image.open(args.n64_frame).convert("RGB")
    port = Image.open(args.port_frame).convert("RGB")
    for label, image, crop in (
        ("N64", n64, args.n64_crop),
        ("port", port, args.port_crop),
    ):
        if not (
            0 <= crop[0] < crop[2] <= image.width
            and 0 <= crop[1] < crop[3] <= image.height
        ):
            raise SystemExit(f"{label} crop {crop} is outside {image.size}")

    board = Image.new("RGB", (2400, 1770), BACKGROUND)
    draw = ImageDraw.Draw(board)
    title_font = font(48, bold=True)
    heading_font = font(31, bold=True)
    body_font = font(27)
    small_font = font(23)

    draw.text(
        (48, 34),
        "Super Dreamland spawn-road source-to-port proof",
        fill=TEXT,
        font=title_font,
    )
    draw.text(
        (50, 94),
        "Full frames provide context; outlined regions are enlarged below.",
        fill=MUTED,
        font=body_font,
    )

    columns = ((45, 1180, SOURCE), (1220, 2355, PORT))
    for x0, x1, color in columns:
        draw.rounded_rectangle((x0, 150, x1, 1380), radius=8, fill=PANEL)
        draw.line((x0, 150, x1, 150), fill=color, width=5)

    draw.text((75, 175), "N64 reference, frame 5460", fill=SOURCE, font=heading_font)
    draw.text((1250, 175), "Corrected Enhanced port", fill=PORT, font=heading_font)

    n64_context = outlined_frame(n64, args.n64_crop, SOURCE)
    port_context = outlined_frame(port, args.port_crop, PORT)
    paste_centered(board, n64_context, (75, 230, 1150, 935))
    paste_centered(board, port_context, (1250, 230, 2325, 935))

    draw.text((75, 962), "Spawn-road region", fill=TEXT, font=body_font)
    draw.text((1250, 962), "Converted spawn-road region", fill=TEXT, font=body_font)
    n64_crop = n64.crop(args.n64_crop)
    port_crop = port.crop(args.port_crop)
    board.paste(
        ImageOps.fit(n64_crop, (1075, 355), Image.Resampling.LANCZOS),
        (75, 1005),
    )
    board.paste(
        ImageOps.fit(port_crop, (1075, 355), Image.Resampling.LANCZOS),
        (1250, 1005),
    )

    state = proof["rdp_depth_render_state"][0]["render_mode"]
    port_trace = proof["port_trace"]
    route_triangles = int(port_trace["route_triangles"])
    footer_top = 1425
    draw.text(
        (55, footer_top),
        "Programmatic identity and render-state evidence",
        fill=TEXT,
        font=heading_font,
    )
    source_line = (
        "Source RDP XRTP 0: opaque Z mode | Z compare ON | Z update ON | "
        f"force blend {'ON' if state['force_blend'] else 'OFF'}"
    )
    port_line = (
        f"Port trace: {route_triangles:,} route triangles | "
        f"{int(port_trace['route_depth_compare_write_triangles']):,} "
        "single-pass Z compare+writes | "
        f"{int(port_trace['route_transparent_triangles']):,} transparent"
    )
    repeat_line = (
        f"Deterministic repeat: {int(repeat['exact_capture_frames'])}/"
        f"{int(repeat['compared_capture_frames'])} frames exact | "
        f"whole-frame and road maximum mean absolute RGB difference "
        f"{float(repeat['max_whole_mean_abs_rgb']):.3f}/255"
    )
    draw.text((55, footer_top + 55), source_line, fill=SOURCE, font=body_font)
    draw.text((55, footer_top + 100), port_line, fill=PORT, font=body_font)
    draw.text((55, footer_top + 145), repeat_line, fill=TEXT, font=body_font)
    draw.text(
        (55, footer_top + 205),
        "The crops show appearance; synchronized RSP/RDP and source-tagged "
        "port traces establish geometry and state.",
        fill=MUTED,
        font=small_font,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    board.save(args.output)
    print(args.output.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
