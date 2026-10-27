#pragma once
/* Pixel routines on a raw 3DS framebuffer (portable C, no Luma dependencies, so they can be
 * unit-tested on a PC against tools/png2fb.py).
 *
 * Framebuffer layout, as written by Luma's drawCharacter():
 *     offset(x, y) = (x * 240 + (239 - y)) * 3,   bytes B, G, R
 * `fbWidth` is 400 for the top screen and 320 for the bottom screen. Everything is clipped.
 *
 * Sprite file "SPR1": 'SPR1', u16 width, u16 height (little endian), then width*height BGRA
 * pixels, row-major, top row first, straight alpha. See tools/png2fb.py.
 */
#include <stdint.h>
#include <stdbool.h>

#define GFX_FB_HEIGHT 240
#define GFX_SPRITE_MAX_DIM 256

/* rgb = 0xRRGGBB (NOT Luma's drawString order). alpha: 0 = transparent .. 255 = opaque. */
void gfx_fb_fill_rect(uint8_t *fb, uint32_t fbWidth, int x, int y, int w, int h,
                      uint32_t rgb, uint32_t alpha);

/* Checks the header and that `size` covers all pixel data. Returns the dimensions via w/h (may be NULL). */
bool gfx_sprite_valid(const uint8_t *data, uint32_t size, uint32_t *w, uint32_t *h);

/* Alpha-blends a (validated) sprite with its top-left corner at (x, y). */
void gfx_fb_blit_sprite(uint8_t *fb, uint32_t fbWidth, int x, int y, const uint8_t *sprite);

/* Draws an 8x8 text string (Luma's font) with its top-left at (x, y), rgb = 0xRRGGBB. Clipped. */
void gfx_fb_text(uint8_t *fb, uint32_t fbWidth, int x, int y, uint32_t rgb, const char *s);

/* Copies the rectangle from the background image `bg` (a full framebuffer of the same size)
 * into `fb`; with bg == NULL it fills the rectangle with `fillRgb` instead. Clipped. */
void gfx_fb_restore_rect(uint8_t *fb, uint32_t fbWidth, int x, int y, int w, int h,
                         const uint8_t *bg, uint32_t fillRgb);