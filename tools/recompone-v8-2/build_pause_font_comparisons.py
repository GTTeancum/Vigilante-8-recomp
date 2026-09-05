"""Render individual typography specimens, not game captures or font deployments."""

from pathlib import Path
import json

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "artifacts" / "pause-font-comparisons"
FONTS = (
    ("rockwell_regular", "Rockwell Regular — current GAME.FNT source family", "ROCK.TTF"),
    ("rockwell_extra_bold", "Rockwell Extra Bold", "ROCKEB.TTF"),
    ("rockwell_condensed_bold", "Rockwell Condensed Bold", "ROCCB___.TTF"),
    ("cooper_black", "Cooper Black", "COOPBL.TTF"),
)


def cap_font(path, height):
    # Equal measured cap height, not misleading differences in nominal size.
    size = min(range(12, 120), key=lambda size: abs(
        (lambda box: box[3] - box[1])(ImageFont.truetype(str(path), size).getbbox("H")) - height))
    return ImageFont.truetype(str(path), size)


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    label_font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 22)
    records = []
    for identifier, label, filename in FONTS:
        path = Path("C:/Windows/Fonts") / filename
        font = cap_font(path, 40)
        image = Image.new("RGB", (1280, 580), (18, 21, 18))
        draw = ImageDraw.Draw(image)
        draw.text((36, 20), label, font=label_font, fill="#ffffff")
        draw.text((36, 52), "TYPE SAMPLE ONLY — not an in-game screenshot; equal 40px capital height",
                  font=label_font, fill="#adb7ad")
        draw.rounded_rectangle((32, 102, 1248, 512), radius=26,
                               fill="#11150f", outline="#d5ab00", width=5)

        def text(value, x, y, color, centered=False):
            box = draw.textbbox((0, 0), value, font=font)
            width = box[2] - box[0]
            if width > 1150:
                raise ValueError(f"Specimen would clip: {identifier}: {value}")
            if centered:
                x -= width / 2
            # Same visible-ink top for every typeface; no scaling or stretching.
            origin = (round(x - box[0]), y - box[1])
            draw.text((origin[0] + 2, origin[1] + 3), value, font=font, fill="#000000")
            draw.text(origin, value, font=font, fill=color)

        text("PAUSED", 640, 151, "#ffef00", centered=True)
        text("Track  <  Return to Houston  >", 640, 264, "#bfc2b8", centered=True)
        text("Quit", 112, 385, "#ffffff")
        text("Resume", 913, 385, "#bfc2b8")
        draw.text((36, 540), "Same layout, colors and wording. Native font-hook spacing/filtering still requires an in-game proof.",
                  font=label_font, fill="#adb7ad")
        target = OUTPUT / f"{identifier}.png"
        image.save(target)
        records.append({"id": identifier, "font": str(path), "size": font.size,
                        "capital_height": 40, "image": str(target), "deployed": False})
        print(target)
    (OUTPUT / "index.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
