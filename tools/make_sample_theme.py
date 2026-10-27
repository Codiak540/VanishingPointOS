#!/usr/bin/env python3
"""Draws the sample theme (PNG sources) into ../theme/. Run png2fb.py on them afterwards,
or just run ../tools/build_theme.sh. Everything is drawn at 4x and downsampled for crisp edges."""
import os
from PIL import Image, ImageDraw, ImageFilter

OUT = os.path.join(os.path.dirname(__file__), "..", "theme")
os.makedirs(OUT, exist_ok=True)
SS = 4  # supersampling factor


def gradient(w, h, top, bottom):
    im = Image.new("RGB", (w, h))
    d = ImageDraw.Draw(im)
    for y in range(h):
        t = y / max(1, h - 1)
        d.line([(0, y), (w, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(top, bottom)))
    return im


def vignette(im, strength=70):
    w, h = im.size
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).ellipse([-w * 0.25, -h * 0.4, w * 1.25, h * 1.4], fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(w // 5))
    dark = Image.new("RGB", (w, h), (0, 0, 0))
    return Image.composite(im, Image.blend(im, dark, strength / 100), mask)


def panel(base, box, radius, fill, outline=None):
    w, h = base.size
    big = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    d = ImageDraw.Draw(big)
    b = [v * SS for v in box]
    d.rounded_rectangle(b, radius * SS, fill=fill, outline=outline, width=SS if outline else 0)
    big = big.resize((w, h), Image.LANCZOS)
    out = base.convert("RGBA")
    out.alpha_composite(big)
    return out.convert("RGB")


# ---- top screen 400x240: header band + list panel -------------------------------
top = vignette(gradient(400, 240, (22, 28, 52), (8, 10, 22)))
top = panel(top, (0, 0, 399, 38), 0, (255, 255, 255, 14))
top = panel(top, (0, 37, 399, 39), 0, (255, 170, 51, 200))          # accent line under header
top = panel(top, (6, 43, 393, 229), 8, (0, 0, 0, 90), (255, 255, 255, 28))  # list panel
top.save(os.path.join(OUT, "top.png"))

# ---- bottom screen 320x240: info panel -------------------------------------------
bot = vignette(gradient(320, 240, (20, 24, 44), (8, 10, 20)))
bot = panel(bot, (8, 8, 311, 111), 8, (0, 0, 0, 90), (255, 255, 255, 28))     # selected-title card
bot = panel(bot, (8, 121, 311, 231), 8, (0, 0, 0, 90), (255, 255, 255, 28))   # controls card
bot.save(os.path.join(OUT, "bottom.png"))

# ---- logo sprite 32x32 with alpha -------------------------------------------------
S = 32 * SS * 2
logo = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(logo)
d.ellipse([S * 0.03, S * 0.03, S * 0.97, S * 0.97], fill=(255, 170, 51, 255))
d.ellipse([S * 0.10, S * 0.10, S * 0.90, S * 0.90], fill=(30, 36, 66, 255))
d.polygon([(S * 0.40, S * 0.28), (S * 0.40, S * 0.72), (S * 0.74, S * 0.50)], fill=(255, 255, 255, 255))
logo.resize((32, 32), Image.LANCZOS).save(os.path.join(OUT, "logo.png"))
print("sample theme written to", os.path.abspath(OUT))
