#include "gfx.h"
#include "gfx_fb.h"
#include "launcher_config.h"
#include "fs.h"
#include "screen.h"

/* Image cache in FCRAM, well clear of the 0x20001000 buffer Luma's FIRM is loaded into. */
#define BG_TOP_ADDR     0x22000000u
#define BG_BOTTOM_ADDR  0x22100000u
#define SPRITE_ADDR     0x22200000u
#define SPRITE_MAX_SIZE (8 + GFX_SPRITE_MAX_DIM * GFX_SPRITE_MAX_DIM * 4)

static u8 *const bgTop    = (u8 *)BG_TOP_ADDR;
static u8 *const bgBottom = (u8 *)BG_BOTTOM_ADDR;
static u8 *const logo     = (u8 *)SPRITE_ADDR;

static bool haveTop, haveBottom, haveLogo;

void gfx_theme_load(void)
{
    /* Backgrounds must be exactly one framebuffer; anything else is ignored. */
    haveTop    = fileRead(bgTop,    THEME_DIR "/top.bin",    SCREEN_TOP_FBSIZE)    == SCREEN_TOP_FBSIZE;
    haveBottom = fileRead(bgBottom, THEME_DIR "/bottom.bin", SCREEN_BOTTOM_FBSIZE) == SCREEN_BOTTOM_FBSIZE;

    u32 n = fileRead(logo, THEME_DIR "/logo.spr", SPRITE_MAX_SIZE);
    haveLogo = n != 0 && gfx_sprite_valid(logo, n, NULL, NULL);
}

const u8 *gfx_bg_top(void)    { return haveTop ? bgTop : NULL; }
const u8 *gfx_bg_bottom(void) { return haveBottom ? bgBottom : NULL; }
const u8 *gfx_logo(void)      { return haveLogo ? logo : NULL; }