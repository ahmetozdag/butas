#!/usr/bin/env python3
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
FONT_PATH = ROOT / "app" / "assets" / "fonts" / "SpaceGrotesk.ttf"
RES = ROOT / "android" / "res"

CHARCOAL = (0x27, 0x27, 0x2A, 255)
WHITE = (255, 255, 255, 255)
TEXT = "BTA"
TRACKING_EM = 0.05

LEGACY_SIZES = {"mdpi": 48, "hdpi": 72, "xhdpi": 96, "xxhdpi": 144, "xxxhdpi": 192}
FOREGROUND_SIZES = {"mdpi": 108, "hdpi": 162, "xhdpi": 216, "xxhdpi": 324, "xxxhdpi": 432}


def text_layer(px: int) -> Image.Image:
    font = ImageFont.truetype(str(FONT_PATH), px)
    try:
        font.set_variation_by_name("Bold")
    except Exception:
        pass
    track = TRACKING_EM * px
    advances = [font.getlength(ch) for ch in TEXT]
    total = sum(advances) + track * (len(TEXT) - 1)
    canvas = Image.new("L", (int(total + px), int(px * 2.2)), 0)
    draw = ImageDraw.Draw(canvas)
    x = px / 2.0
    for ch, adv in zip(TEXT, advances):
        draw.text((x, px * 1.5), ch, font=font, fill=255, anchor="ls")
        x += adv + track
    bbox = canvas.getbbox()
    return canvas.crop(bbox)


def make_legacy(size: int, round_shape: bool = False) -> Image.Image:
    ss = 4
    s = size * ss
    base = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    draw = ImageDraw.Draw(base)
    if round_shape:
        draw.ellipse((0, 0, s - 1, s - 1), fill=CHARCOAL)
    else:
        draw.rounded_rectangle((0, 0, s - 1, s - 1), radius=int(s * 0.22), fill=CHARCOAL)

    glyphs = text_layer(400)
    width = int(s * 0.58)
    height = max(1, int(glyphs.height * width / glyphs.width))
    glyphs = glyphs.resize((width, height), Image.LANCZOS)
    layer = Image.new("RGBA", glyphs.size, WHITE[:3] + (0,))
    layer.putalpha(glyphs)
    base.alpha_composite(layer, ((s - width) // 2, (s - height) // 2))
    return base.resize((size, size), Image.LANCZOS)


def make_foreground(size: int) -> Image.Image:
    ss = 4
    s = size * ss
    base = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    glyphs = text_layer(400)
    width = int(s * 0.54)
    height = max(1, int(glyphs.height * width / glyphs.width))
    glyphs = glyphs.resize((width, height), Image.LANCZOS)
    layer = Image.new("RGBA", glyphs.size, WHITE[:3] + (0,))
    layer.putalpha(glyphs)
    base.alpha_composite(layer, ((s - width) // 2, (s - height) // 2))
    return base.resize((size, size), Image.LANCZOS)


ADAPTIVE_XML = """<?xml version="1.0" encoding="utf-8"?>
<adaptive-icon xmlns:android="http://schemas.android.com/apk/res/android">
    <background android:drawable="@color/ic_launcher_background" />
    <foreground android:drawable="@mipmap/ic_launcher_foreground" />
</adaptive-icon>
"""

COLORS_XML = """<?xml version="1.0" encoding="utf-8"?>
<resources>
    <color name="ic_launcher_background">#27272A</color>
</resources>
"""


def main() -> None:
    (RES / "values").mkdir(parents=True, exist_ok=True)
    (RES / "mipmap-anydpi-v26").mkdir(parents=True, exist_ok=True)

    for dpi, size in LEGACY_SIZES.items():
        d = RES / f"mipmap-{dpi}"
        d.mkdir(parents=True, exist_ok=True)
        make_legacy(size).save(d / "ic_launcher.png")
        make_legacy(size, round_shape=True).save(d / "ic_launcher_round.png")

    for dpi, size in FOREGROUND_SIZES.items():
        d = RES / f"mipmap-{dpi}"
        d.mkdir(parents=True, exist_ok=True)
        make_foreground(size).save(d / "ic_launcher_foreground.png")

    (RES / "mipmap-anydpi-v26" / "ic_launcher.xml").write_text(ADAPTIVE_XML, encoding="utf-8")
    (RES / "mipmap-anydpi-v26" / "ic_launcher_round.xml").write_text(ADAPTIVE_XML, encoding="utf-8")
    (RES / "values" / "colors.xml").write_text(COLORS_XML, encoding="utf-8")
    print(f"Ikonlar uretildi: {RES}")


if __name__ == "__main__":
    main()
