#pragma once
#include "types.h"

/* Loads THEME_DIR/top.bin, bottom.bin and logo.spr if present (see tools/png2fb.py).
   Missing files are fine: that part just falls back to a flat background. */
void gfx_theme_load(void);

/* Pointers into the loaded images, or NULL when a file is missing/invalid. */
const u8 *gfx_bg_top(void);
const u8 *gfx_bg_bottom(void);
const u8 *gfx_logo(void);