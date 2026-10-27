#!/usr/bin/env python3
"""Renders what the FIRM's menu will look like, using Luma's real font and the same layout
numbers as drawMenu() in main.c (so layout mistakes show up on a PC). Output: preview_*.png"""
import re, sys, os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
font = bytes(int(v, 16) for v in re.findall(r"0x([0-9a-fA-F]{2})", open(os.path.join(ROOT, "arm9/source/font.h")).read()))

def mix(s, d, a):
    t = s * a + d * (255 - a) + 128
    return (t + (t >> 8)) >> 8

def text(img, x, y, rgb, s):
    px = img.load()
    line_i = 0
    for ch in s:
        for yy in range(8):
            bits = font[ord(ch) * 8 + yy]
            for xx in range(8):
                if (bits >> (7 - xx)) & 1:
                    X, Y = x + line_i * 8 + xx, y + yy
                    if 0 <= X < img.width and 0 <= Y < img.height:
                        px[X, Y] = rgb
        line_i += 1

def rect(img, x, y, w, h, rgb, a):
    px = img.load()
    for X in range(max(0, x), min(img.width, x + w)):
        for Y in range(max(0, y), min(img.height, y + h)):
            p = px[X, Y]
            px[X, Y] = tuple(mix(c, d, a) for c, d in zip(rgb, p))

ACCENT, TXT, DIM, WARN = (255, 170, 51), (236, 239, 246), (150, 158, 180), (255, 96, 96)
titles = [("The Legend of Zelda: Ocarina of Time 3D", "sd"), ("Animal Crossing: New Leaf", "sd"),
          ("Mario Kart 7", "sd"), ("Fire Emblem Awakening", "sd"), ("Pokemon X", "game"),
          ("Super Smash Bros. for Nintendo 3DS", "sd"), ("Metroid: Samus Returns", "sd"),
          ("A very long title that goes past the column limit of 40", "sd")] + [(f"Game number {i}", "sd") for i in range(9, 20)]
sel, top, ROWS = 1, 0, 17

t_img = Image.open(os.path.join(ROOT, "theme/top.png")).convert("RGB")
b_img = Image.open(os.path.join(ROOT, "theme/bottom.png")).convert("RGB")
logo = Image.open(os.path.join(ROOT, "theme/logo.png")).convert("RGBA")
t_img.paste(logo, (8, 3), logo)
text(t_img, 48, 14, ACCENT, "Vanishing Point OS - V0.2")
text(t_img, 340, 14, DIM, f"{sel+1}/{len(titles)}")
for r in range(ROWS):
    if top + r >= len(titles): break
    name, media = titles[top + r]
    y = 50 + r * 10
    s = top + r == sel
    if s: rect(t_img, 12, y - 2, 376, 12, ACCENT, 80)
    text(t_img, 20, y, TXT if s else DIM, name[:40])
    text(t_img, 352, y, ACCENT if s else DIM, media)

name, media = titles[sel]
text(b_img, 16, 16, TXT, name[:36]);  text(b_img, 16, 28, TXT, name[36:72])
text(b_img, 16, 52, DIM, "ID    0004000000" + "0A1B2C"[:6]); text(b_img, 16, 64, DIM, f"Media {media}")
for i, (k, v) in enumerate([("Up/Down", "move"), ("L/R", "page"), ("A", "launch"), ("B", "boot Luma normally"), ("Y", "rescan titles")]):
    text(b_img, 16, 132 + 14 * i, ACCENT, k); text(b_img, 112, 132 + 14 * i, DIM, v)

t_img.resize((800, 480), Image.NEAREST).save(os.path.join(ROOT, "preview_top.png"))
b_img.resize((640, 480), Image.NEAREST).save(os.path.join(ROOT, "preview_bottom.png"))
print("wrote preview_top.png / preview_bottom.png (2x)")
