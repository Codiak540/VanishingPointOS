#include "gfx_fb.h"
#include "font.h"
#include <string.h>

/* round(src*a/255 + dst*(255-a)/255), exact for all 8-bit inputs */
static inline uint8_t mix(uint32_t src, uint32_t dst, uint32_t a)
{
    uint32_t t = src * a + dst * (255 - a) + 128;
    return (uint8_t)((t + (t >> 8)) >> 8);
}

static inline uint8_t *pixel(uint8_t *fb, int x, int y)
{
    return fb + ((uint32_t)x * GFX_FB_HEIGHT + (uint32_t)(GFX_FB_HEIGHT - 1 - y)) * 3;
}

void gfx_fb_fill_rect(uint8_t *fb, uint32_t fbWidth, int x, int y, int w, int h,
                      uint32_t rgb, uint32_t alpha)
{
    if(alpha == 0 || w <= 0 || h <= 0) return;
    if(alpha > 255) alpha = 255;

    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > (int)fbWidth ? (int)fbWidth : x + w;
    int y1 = y + h > GFX_FB_HEIGHT ? GFX_FB_HEIGHT : y + h;

    uint32_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;

    for(int xx = x0; xx < x1; xx++)
        for(int yy = y0; yy < y1; yy++)
        {
            uint8_t *p = pixel(fb, xx, yy);
            if(alpha == 255) { p[0] = (uint8_t)b; p[1] = (uint8_t)g; p[2] = (uint8_t)r; }
            else { p[0] = mix(b, p[0], alpha); p[1] = mix(g, p[1], alpha); p[2] = mix(r, p[2], alpha); }
        }
}

bool gfx_sprite_valid(const uint8_t *d, uint32_t size, uint32_t *w, uint32_t *h)
{
    if(size < 8 || d[0] != 'S' || d[1] != 'P' || d[2] != 'R' || d[3] != '1') return false;

    uint32_t sw = (uint32_t)d[4] | ((uint32_t)d[5] << 8);
    uint32_t sh = (uint32_t)d[6] | ((uint32_t)d[7] << 8);
    if(sw == 0 || sh == 0 || sw > GFX_SPRITE_MAX_DIM || sh > GFX_SPRITE_MAX_DIM) return false;
    if(size < 8 + sw * sh * 4) return false;

    if(w) *w = sw;
    if(h) *h = sh;
    return true;
}

void gfx_fb_blit_sprite(uint8_t *fb, uint32_t fbWidth, int x, int y, const uint8_t *s)
{
    uint32_t sw = (uint32_t)s[4] | ((uint32_t)s[5] << 8);
    uint32_t sh = (uint32_t)s[6] | ((uint32_t)s[7] << 8);
    const uint8_t *px = s + 8;

    for(uint32_t sy = 0; sy < sh; sy++)
    {
        int yy = y + (int)sy;
        if(yy < 0 || yy >= GFX_FB_HEIGHT) continue;

        for(uint32_t sx = 0; sx < sw; sx++)
        {
            int xx = x + (int)sx;
            if(xx < 0 || xx >= (int)fbWidth) continue;

            const uint8_t *q = px + (sy * sw + sx) * 4;   /* B G R A */
            uint32_t a = q[3];
            if(a == 0) continue;

            uint8_t *p = pixel(fb, xx, yy);
            if(a == 255) { p[0] = q[0]; p[1] = q[1]; p[2] = q[2]; }
            else { p[0] = mix(q[0], p[0], a); p[1] = mix(q[1], p[1], a); p[2] = mix(q[2], p[2], a); }
        }
    }
}

void gfx_fb_text(uint8_t *fb, uint32_t fbWidth, int x, int y, uint32_t rgb, const char *s)
{
    uint8_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;

    for(; *s; s++, x += 8)
    {
        if(x + 8 <= 0) continue;
        if(x >= (int)fbWidth) break;

        const unsigned char *glyph = &font[(unsigned char)*s * 8];

        for(int yy = 0; yy < 8; yy++)
        {
            int Y = y + yy;
            if(Y < 0 || Y >= GFX_FB_HEIGHT) continue;

            for(int xx = 0; xx < 8; xx++)
            {
                int X = x + xx;
                if(X < 0 || X >= (int)fbWidth) continue;
                if(!((glyph[yy] >> (7 - xx)) & 1)) continue;

                uint8_t *p = pixel(fb, X, Y);
                p[0] = b; p[1] = g; p[2] = r;
            }
        }
    }
}

void gfx_fb_restore_rect(uint8_t *fb, uint32_t fbWidth, int x, int y, int w, int h,
                         const uint8_t *bg, uint32_t fillRgb)
{
    if(w <= 0 || h <= 0) return;

    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > (int)fbWidth ? (int)fbWidth : x + w;
    int y1 = y + h > GFX_FB_HEIGHT ? GFX_FB_HEIGHT : y + h;
    if(x0 >= x1 || y0 >= y1) return;

    if(bg && x0 == 0 && y0 == 0 && x1 == (int)fbWidth && y1 == GFX_FB_HEIGHT)
    {
        memcpy(fb, bg, (size_t)fbWidth * GFX_FB_HEIGHT * 3);
        return;
    }

    uint8_t r = (fillRgb >> 16) & 0xFF, g = (fillRgb >> 8) & 0xFF, b = fillRgb & 0xFF;

    for(int xx = x0; xx < x1; xx++)
    {
        /* one column, rows y0..y1-1: contiguous in memory (rows run bottom-to-top) */
        uint32_t off = ((uint32_t)xx * GFX_FB_HEIGHT + (uint32_t)(GFX_FB_HEIGHT - y1)) * 3;
        uint32_t len = (uint32_t)(y1 - y0) * 3;

        if(bg) memcpy(fb + off, bg + off, len);
        else for(uint32_t i = 0; i < len; i += 3) { fb[off + i] = b; fb[off + i + 1] = g; fb[off + i + 2] = r; }
    }
}