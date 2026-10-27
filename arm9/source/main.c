/*
*   Vanishing Point OS: ARM9 side.
*
*   Built from parts of Luma3DS (GPLv3): start.s, linker script, screen/draw/i2c/
*   utils/chainloader code and the FatFS port are Luma's; this file and fs.c are
*   new. See LICENSE / README.md.
*/

#include "types.h"
#include "launcher_config.h"
#include "fs.h"
#include "screen.h"
#include "draw.h"
#include "i2c.h"
#include "utils.h"
#include "buttons.h"
#include "chainloader.h"
#include "memory.h"
#include "fmt.h"
#include "titles.h"
#include "gfx.h"
#include "menu_view.h"

extern u8 __itcm_start__[], __itcm_lma__[], __itcm_bss_start__[], __itcm_end__[];

/* Referenced by Luma's utils.c / screen.c */
BootType bootType = B9S;

/* The firm buffer address and limit Luma itself uses for external FIRMs. */
static Firm *const firm = (Firm *)0x20001000;
#define FIRM_BUFFER_END 0x27FFE000u

static char lumaLaunchPath[64] = "sdmc:/boot.firm";

/* ---- FIRM sanity check (Luma's checkFirm minus the SHA-256 verification) ---- */

static __attribute__((noinline)) bool overlaps(u32 as, u32 ae, u32 bs, u32 be)
{
    if(as <= bs && bs <= ae) return true;
    if(bs <= as && as <= be) return true;
    return false;
}

static __attribute__((noinline)) bool inRange(u32 as, u32 ae, u32 bs, u32 be)
{
    return as >= bs && ae <= be;
}

static bool checkFirm(u32 firmSize)
{
    if(memcmp(firm->magic, "FIRM", 4) != 0 || firm->arm9Entry == NULL) return false;

    bool arm9EpFound = false, arm11EpFound = false;

    u32 size = 0x200;
    for(u32 i = 0; i < 4; i++) size += firm->section[i].size;
    if(firmSize < size) return false;

    for(u32 i = 0; i < 4; i++)
    {
        FirmSection *s = &firm->section[i];
        if(s->size == 0) continue;

        if((s->offset < 0x200) ||
           (s->address + s->size < s->address) ||
           ((u32)s->address & 3) || (s->offset & 0x1FF) || (s->size & 0x1FF) ||
           overlaps((u32)s->address, (u32)s->address + s->size, (u32)firm, (u32)firm + size) ||
           ((!inRange((u32)s->address, (u32)s->address + s->size, 0x08000000, 0x08000000 + 0x00100000)) &&
            (!inRange((u32)s->address, (u32)s->address + s->size, 0x18000000, 0x18000000 + 0x00600000)) &&
            (!inRange((u32)s->address, (u32)s->address + s->size, 0x1FF00000, 0x1FFFFC00)) &&
            (!inRange((u32)s->address, (u32)s->address + s->size, 0x20000000, 0x20000000 + 0x8000000))))
            return false;

        if(firm->arm9Entry >= s->address && firm->arm9Entry < (s->address + s->size)) arm9EpFound = true;
        if(firm->arm11Entry >= s->address && firm->arm11Entry < (s->address + s->size)) arm11EpFound = true;
    }

    return arm9EpFound && (firm->arm11Entry == NULL || arm11EpFound);
}

/* ---- UI helpers ---- */

#if DEBUG_STEPS
static u32 line;
static void say(u32 color, const char *s)
{
    drawString(true, 10, line, color, s);
    line += SPACING_Y * 2;
}
#endif

/* ---- Hand off to the real Luma3DS boot.firm ---- */

/* Which fbs[] index is on screen. The menu double-buffers (see "double-buffered presentation"
   below); Luma's error()/drawString() always draw into fbs[0], so error paths call
   showFront0() first. */
static int shown = 0;

/* Luma's error()/drawString() draw into fbs[0]; make sure that is the one on screen. */
static void showFront0(void)
{
    if(shown != 0)
    {
        swapFramebuffers(false);
        shown = 0;
    }
}

static void bootLuma(bool waitForRelease)
{
    u32 size = fileRead(firm, LUMA_FIRM_PATH, FIRM_BUFFER_END - (u32)firm);

    if(size == 0)
    {
        showFront0();
        error("Could not read Luma from:\n" LUMA_FIRM_PATH "\n(missing, unreadable or too big)");
    }
    if(size <= 0x200 || !checkFirm(size))
    {
        showFront0();
        error("Not a valid FIRM:\n" LUMA_FIRM_PATH "\nsize: %u bytes", (unsigned)size);
    }

#if DEBUG_STEPS
    initScreens();
    line = 10;
    say(COLOR_TITLE, "Handoff debug");
    drawFormattedString(true, 10, line, COLOR_WHITE, "FIRM size: %u", (unsigned)size); line += SPACING_Y;
    drawFormattedString(true, 10, line, COLOR_WHITE, "ARM9 entry:  %08X", (unsigned)(u32)firm->arm9Entry); line += SPACING_Y;
    drawFormattedString(true, 10, line, COLOR_WHITE, "ARM11 entry: %08X", (unsigned)(u32)firm->arm11Entry); line += SPACING_Y;
    for(u32 i = 0; i < 4; i++)
    {
        drawFormattedString(true, 10, line, COLOR_WHITE, "sec%u: %08X size %06X", (unsigned)i,
                            (unsigned)(u32)firm->section[i].address, (unsigned)firm->section[i].size);
        line += SPACING_Y;
    }
    line += SPACING_Y;
    say(COLOR_YELLOW, "Press A to chainload");
    while(!(waitInput(false) & BUTTON_A));
#endif

    /* Luma reads the held buttons as boot combos, so let go of A/B first
       (but not when the user is deliberately holding a combo to reach Luma). */
    if(waitForRelease)
        while(HID_PAD & PIN_BUTTONS);

    /* Present ourselves to Luma as a normal boot9strap launch of /boot.firm
       (Luma checks argv[0] starts with "sdmc" and the 0xBEEF magic that
       chainload() supplies). */
    char *argv[1] = { lumaLaunchPath };

    unmountPartitions();            /* Luma re-initialises the SD card itself */
    prepareArm11ForFirmlaunch();    /* parks the ARM11 core so it can be re-entered */
    chainload(1, argv, firm);       /* runs from ITCM; does not return */
    __builtin_unreachable();
}

/* ---- Title menu ---- */

#define MENU_ROWS 17


static char  titlesText[24 * 1024];
static Title titles[TITLES_MAX];
static int   titleCount;
static bool  usingFallback;

static void loadTitles(void)
{
    u32 got = fileRead(titlesText, TITLES_FILE, sizeof(titlesText) - 1);
    titleCount = got != 0 ? titles_parse(titlesText, got, titles, TITLES_MAX) : 0;
    usingFallback = false;

    if(titleCount == 0)
    {
        static const char fallback[] = TARGET_TID ":" TARGET_MEDIA ":(built-in test entry)\n";
        titleCount = titles_parse(fallback, sizeof(fallback) - 1, titles, 1);
        usingFallback = true;
    }

    if(titleCount == 0)   /* TARGET_TID itself is malformed */
        error("No usable title list and TARGET_TID is invalid.");
}

/* ---- double-buffered presentation ----
 * The menu is drawn into the framebuffer that is NOT on screen and then swapped in, so the
 * screen never shows a half-drawn frame. `shown` is the framebuffer index currently visible
 * (initScreens() leaves fbs[0] showing). mv_render() redraws only what changed. */

static MenuView view;

static void presentMenu(int sel, int top)
{
    int back = 1 - shown;

    mv_render(&view, back, sel, top, titleCount, usingFallback);
    swapFramebuffers(back == 1);
    shown = back;
}

static void setupView(void)
{
    MvSurface s =
    {
        .top    = { fbs[0].top_left, fbs[1].top_left },
        .bottom = { fbs[0].bottom,   fbs[1].bottom },
        .bgTop = gfx_bg_top(),
        .bgBottom = gfx_bg_bottom(),
        .logo = gfx_logo(),
    };
    mv_init(&view, &s, titles);
}

static void launchTitle(const Title *t)
{
    char req[32];
    size_t n = titles_format_request(req, sizeof(req), t->tid, t->media);

    if(n == 0 || !writeFileAtomic(TID_DIR, TID_FILE, req, (u32)n))
    {
        showFront0();
        error("Failed to write " TID_DIR "/" TID_FILE);
    }

    mv_message(&view, shown, "Booting Luma...", true);
    bootLuma(true);
}

static void requestScan(void)
{
    static const char one[] = "1\n";

    if(!writeFileAtomic(TID_DIR, SCAN_REQ_FILE, one, sizeof(one) - 1))
    {
        showFront0();
        error("Failed to write " TID_DIR "/" SCAN_REQ_FILE);
    }

    mv_message(&view, shown, "Scan requested. Booting Luma...", true);
    bootLuma(true);
}

static void runMenu(void)
{
    int sel = 0, top = 0;

    loadTitles();

    for(;;)
    {
        presentMenu(sel, top);
        u32 key = waitInput(true);   /* true = d-pad auto-repeat */

        if(key & BUTTON_UP)                          sel = sel > 0 ? sel - 1 : titleCount - 1;
        else if(key & BUTTON_DOWN)                   sel = sel < titleCount - 1 ? sel + 1 : 0;
        else if(key & (BUTTON_LEFT | BUTTON_L1))     sel = sel - MENU_ROWS > 0 ? sel - MENU_ROWS : 0;
        else if(key & (BUTTON_RIGHT | BUTTON_R1))    sel = sel + MENU_ROWS < titleCount ? sel + MENU_ROWS : titleCount - 1;
        else if(key & BUTTON_A)                      launchTitle(&titles[sel]);
        else if(key & BUTTON_B)                      bootLuma(true);
        else if(key & BUTTON_Y)                      requestScan();

        if(sel < top) top = sel;
        if(sel >= top + MENU_ROWS) top = sel - MENU_ROWS + 1;
    }
}

void main(int argc, char **argv, u32 magicWord)
{
    /* Copy the launch path first: the ITCM setup below can clobber the area argv lives in. */
    if(argc >= 1 && (magicWord & 0xFFFF) == 0xBEEF && argv != NULL && argv[0] != NULL)
    {
        u32 i;
        for(i = 0; i < sizeof(lumaLaunchPath) - 1 && argv[0][i] != 0; i++)
            lumaLaunchPath[i] = argv[0][i];
        lumaLaunchPath[i] = 0;
    }

    /* Load the ITCM-resident code (chainloader, I2C) and zero its bss. */
    memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
    memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);
    I2C_init();

    if(!mountSd())
    {
        initScreens();
        error("Failed to mount the SD card.");
    }

    /* Coming back from the companion's reboot-into-title: no menu, straight to Luma. */
    if(fileConsume(LOCK_FILE)) bootLuma(false);
#if AUTO_HANDOFF_ON_FIRMLAUNCH
    if(magicWord == 0xBABE) bootLuma(false);
#endif

    /* Keep Luma's boot-key combos (Select = config, Start = chainloader,
       emuNAND/payload hotkeys, ...) working: if anything is held, skip our UI. */
    if(HID_PAD & PIN_BUTTONS) bootLuma(false);

    initScreens();
    gfx_theme_load();
    setupView();
    runMenu();
}