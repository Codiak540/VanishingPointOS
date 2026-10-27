#pragma once
/* The menu's renderer (portable C, so it can be tested on a PC).
 *
 * It owns no hardware: it draws into two framebuffers per screen. The caller shows one buffer
 * while mv_render() draws into the other, then swaps. Each buffer remembers what it currently
 * shows, so a frame only redraws what differs from that buffer's last contents (typically the
 * two rows whose highlight changed, plus the counter and the info card). The static parts
 * (backgrounds, logo, titles, control hints) are drawn once per buffer. */
#include <stdint.h>
#include <stdbool.h>
#include "titles.h"

#define MV_ROWS 17

typedef struct
{
    uint8_t *top[2];          /* the two top-screen framebuffers (400x240) */
    uint8_t *bottom[2];       /* the two bottom-screen framebuffers (320x240) */
    const uint8_t *bgTop;     /* theme backgrounds, or NULL for a flat colour */
    const uint8_t *bgBottom;
    const uint8_t *logo;      /* a validated "SPR1" sprite, or NULL */
} MvSurface;

typedef struct
{
    bool valid;
    int rowTitle[MV_ROWS];
    bool rowSel[MV_ROWS];
    int sel, count;
    bool fallback;
} MvBufState;

typedef struct
{
    MvSurface s;
    const Title *titles;
    MvBufState st[2];
    /* statistics, used by the tests */
    unsigned fullDraws, rowsDrawn;
} MenuView;

void mv_init(MenuView *v, const MvSurface *surface, const Title *titles);

/* Renders the menu state into buffer `b` (0 or 1). */
void mv_render(MenuView *v, int b, int sel, int top, int count, bool fallback);

/* Draws a one-line status message on the bottom screen of buffer `b` (marks that buffer stale). */
void mv_message(MenuView *v, int b, const char *msg, bool good);