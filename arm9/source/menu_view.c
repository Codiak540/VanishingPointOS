#include "menu_view.h"
#include "gfx_fb.h"
#include <string.h>

#define TOP_W 400
#define BOT_W 320

#define ACCENT 0xFFAA33u
#define TEXT   0xECEFF6u
#define DIM    0x969EB4u
#define WARN   0xFF6060u
#define OK     0x6EDC82u
#define FLAT_TOP    0x0C0E1Au
#define FLAT_BOTTOM 0x0C0E1Au

/* layout (pixels) */
#define ROW_X     12
#define ROW_W     376
#define ROW_Y0    49          /* band of row r: y = 49 + 10*r, height 10 (bands never overlap) */
#define ROW_PITCH 10
#define NAME_X    20
#define MEDIA_X   352
#define NAME_MAX_COLS 40

void mv_init(MenuView *v, const MvSurface *surface, const Title *titles)
{
    memset(v, 0, sizeof(*v));
    v->s = *surface;
    v->titles = titles;
}

static int putUint(char *out, unsigned v)
{
    char tmp[12];
    int n = 0, len = 0;
    do { tmp[n++] = (char)('0' + v % 10); v /= 10; } while(v);
    while(n) out[len++] = tmp[--n];
    return len;
}

static void putHex64(char *out, uint64_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    for(int i = 0; i < 16; i++) out[i] = hex[(v >> (60 - 4 * i)) & 0xF];
}

static void restoreTop(const MenuView *v, uint8_t *fb, int x, int y, int w, int h)
{
    gfx_fb_restore_rect(fb, TOP_W, x, y, w, h, v->s.bgTop, FLAT_TOP);
}

static void restoreBottom(const MenuView *v, uint8_t *fb, int x, int y, int w, int h)
{
    gfx_fb_restore_rect(fb, BOT_W, x, y, w, h, v->s.bgBottom, FLAT_BOTTOM);
}

/* ---- parts that never change: drawn once per buffer ---- */
static void drawStatic(const MenuView *v, uint8_t *top, uint8_t *bottom)
{
    restoreTop(v, top, 0, 0, TOP_W, 240);
    restoreBottom(v, bottom, 0, 0, BOT_W, 240);

    int titleX = 10;
    if(v->s.logo)
    {
        gfx_fb_blit_sprite(top, TOP_W, 8, 3, v->s.logo);
        titleX = 48;
    }
    gfx_fb_text(top, TOP_W, titleX, 14, ACCENT, "Vanishing Point OS - v0.2");

    static const char *const keys[]  = { "Up/Down", "L/R", "A", "B", "Y" };
    static const char *const hints[] = { "move", "page", "launch", "boot Luma normally", "rescan titles" };
    for(int i = 0; i < 5; i++)
    {
        gfx_fb_text(bottom, BOT_W, 16, 132 + 14 * i, ACCENT, keys[i]);
        gfx_fb_text(bottom, BOT_W, 112, 132 + 14 * i, DIM, hints[i]);
    }
}

/* ---- parts that change ---- */
static void drawRow(MenuView *v, uint8_t *top, int r, int idx, bool selected)
{
    int y = ROW_Y0 + ROW_PITCH * r;
    restoreTop(v, top, ROW_X, y, ROW_W, ROW_PITCH);
    v->rowsDrawn++;
    if(idx < 0) return;

    if(selected) gfx_fb_fill_rect(top, TOP_W, ROW_X, y, ROW_W, ROW_PITCH, ACCENT, 80);

    const Title *t = &v->titles[idx];
    char name[NAME_MAX_COLS + 1];
    int n = 0;
    while(n < NAME_MAX_COLS && t->name[n]) { name[n] = t->name[n]; n++; }
    name[n] = '\0';

    gfx_fb_text(top, TOP_W, NAME_X, y + 1, selected ? TEXT : DIM, name);
    gfx_fb_text(top, TOP_W, MEDIA_X, y + 1, selected ? ACCENT : DIM, titles_media_str(t->media));
}

static void drawCounter(const MenuView *v, uint8_t *top, int sel, int count)
{
    restoreTop(v, top, 336, 12, 60, 10);

    char buf[24];
    int n = putUint(buf, (unsigned)(sel + 1));
    buf[n++] = '/';
    n += putUint(buf + n, (unsigned)count);
    buf[n] = '\0';
    gfx_fb_text(top, TOP_W, 340, 14, DIM, buf);
}

static void drawInfo(const MenuView *v, uint8_t *bottom, int sel, bool fallback)
{
    restoreBottom(v, bottom, 12, 12, 296, 92);

    const Title *cur = &v->titles[sel];
    char l1[40], l2[40];
    int n = 0, m = 0;
    while(n < 36 && cur->name[n]) { l1[n] = cur->name[n]; n++; }
    l1[n] = '\0';
    while(m < 36 && cur->name[n + m]) { l2[m] = cur->name[n + m]; m++; }
    l2[m] = '\0';

    gfx_fb_text(bottom, BOT_W, 16, 16, TEXT, l1);
    if(m) gfx_fb_text(bottom, BOT_W, 16, 28, TEXT, l2);

    char id[32] = "ID    ";
    putHex64(id + 6, cur->tid);
    id[22] = '\0';
    gfx_fb_text(bottom, BOT_W, 16, 52, DIM, id);

    char media[24] = "Media ";
    strcpy(media + 6, titles_media_str(cur->media));
    gfx_fb_text(bottom, BOT_W, 16, 64, DIM, media);

    if(fallback) gfx_fb_text(bottom, BOT_W, 16, 88, WARN, "No title list yet. Press Y to scan.");
}

void mv_render(MenuView *v, int b, int sel, int top, int count, bool fallback)
{
    uint8_t *ft = v->s.top[b], *fbm = v->s.bottom[b];
    MvBufState *st = &v->st[b];
    bool full = !st->valid;

    if(full) { drawStatic(v, ft, fbm); v->fullDraws++; }

    for(int r = 0; r < MV_ROWS; r++)
    {
        int idx = top + r < count ? top + r : -1;
        bool selected = idx >= 0 && idx == sel;

        if(full || st->rowTitle[r] != idx || st->rowSel[r] != selected)
        {
            drawRow(v, ft, r, idx, selected);
            st->rowTitle[r] = idx;
            st->rowSel[r] = selected;
        }
    }

    if(full || st->sel != sel || st->count != count) drawCounter(v, ft, sel, count);
    if(full || st->sel != sel || st->fallback != fallback) drawInfo(v, fbm, sel, fallback);

    st->sel = sel;
    st->count = count;
    st->fallback = fallback;
    st->valid = true;
}

void mv_message(MenuView *v, int b, const char *msg, bool good)
{
    restoreBottom(v, v->s.bottom[b], 12, 204, 296, 16);
    gfx_fb_text(v->s.bottom[b], BOT_W, 16, 210, good ? OK : WARN, msg);
    v->st[b].valid = false;      /* it no longer matches what a fresh render would show */
}