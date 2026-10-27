#!/usr/bin/env python3
"""
Convert a PNG (or anything Pillow can open) into the formats the FIRM's menu loads.

  png2fb.py top.png    top.bin     --top      400x240 background  -> raw top-screen framebuffer
  png2fb.py bottom.png bottom.bin  --bottom   320x240 background  -> raw bottom-screen framebuffer
  png2fb.py logo.png   logo.spr    --sprite   up to 256x256, keeps alpha

The 3DS framebuffer is column-major and upside-down, BGR byte order:
    offset(x, y) = (x * 240 + (239 - y)) * 3      bytes: B, G, R
(this is exactly the layout Luma's own drawCharacter() writes).

Sprite file ("SPR1"): 'SPR1', u16 width, u16 height (little endian), then width*height
BGRA pixels, row-major, top row first, straight (non-premultiplied) alpha.

Backgrounds with transparency are composited over black. Wrong-sized backgrounds are an
error unless --resize is given (then they are scaled with Lanczos).

SVG: rasterise first at the exact size, e.g.
    rsvg-convert -w 400 -h 240 top.svg -o top.png
    inkscape top.svg -w 400 -h 240 -o top.png
"""
import argparse
import struct
import sys

from PIL import Image

SIZES = {"top": (400, 240), "bottom": (320, 240)}


def to_framebuffer(im, width, height):
    rgb = im.convert("RGB")
    px = rgb.load()
    out = bytearray(width * height * 3)
    for x in range(width):
        for y in range(height):
            r, g, b = px[x, y]
            o = (x * height + (height - 1 - y)) * 3
            out[o] = b
            out[o + 1] = g
            out[o + 2] = r
    return bytes(out)


def to_sprite(im):
    rgba = im.convert("RGBA")
    w, h = rgba.size
    if not (1 <= w <= 256 and 1 <= h <= 256):
        sys.exit(f"sprite must be 1..256 px in each direction, got {w}x{h}")
    px = rgba.load()
    out = bytearray(b"SPR1" + struct.pack("<HH", w, h))
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            out += bytes((b, g, r, a))
    return bytes(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    kind = ap.add_mutually_exclusive_group(required=True)
    kind.add_argument("--top", action="store_true")
    kind.add_argument("--bottom", action="store_true")
    kind.add_argument("--sprite", action="store_true")
    ap.add_argument("--resize", action="store_true", help="scale backgrounds to the screen size")
    args = ap.parse_args()

    im = Image.open(args.input)

    if args.sprite:
        data = to_sprite(im)
    else:
        w, h = SIZES["top" if args.top else "bottom"]
        if im.size != (w, h):
            if not args.resize:
                sys.exit(f"{args.input} is {im.size[0]}x{im.size[1]}, expected {w}x{h} (use --resize to scale)")
            im = im.convert("RGBA").resize((w, h), Image.LANCZOS)
        if im.mode in ("RGBA", "LA", "P"):
            im = im.convert("RGBA")
            bg = Image.new("RGBA", im.size, (0, 0, 0, 255))
            bg.alpha_composite(im)
            im = bg
        data = to_framebuffer(im, w, h)

    with open(args.output, "wb") as f:
        f.write(data)
    print(f"{args.input} -> {args.output} ({len(data)} bytes)")


if __name__ == "__main__":
    main()
