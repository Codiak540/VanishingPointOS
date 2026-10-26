/*
*   TID launcher PoC: ARM9 side.
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

static u32 line;
static void say(u32 color, const char *s)
{
    drawString(true, 10, line, color, s);
    line += SPACING_Y * 2;
}

static void showScreen(void)
{
    initScreens();
    line = 10;
    say(COLOR_TITLE, "Vanishing Point OS - v0.1");
    line += SPACING_Y;
    // say(COLOR_WHITE, "A: launch " TARGET_TID " (" TARGET_MEDIA ")");
    say(COLOR_WHITE, "A: Launch \"Ocarina Of Time 3D\"");
    say(COLOR_WHITE, "B: Boot Luma normally");
}

/* ---- Hand off to the real Luma3DS boot.firm ---- */

static void bootLuma(void)
{
    u32 size = fileRead(firm, LUMA_FIRM_PATH, FIRM_BUFFER_END - (u32)firm);

    if(size <= 0x200 || !checkFirm(size))
    {
        error("Could not load Luma from:\n" LUMA_FIRM_PATH);
    }

    /* Present ourselves to Luma as a normal boot9strap launch of /boot.firm
       (Luma checks argv[0] starts with "sdmc" and the 0xBEEF magic that
       chainload() supplies). */
    char *argv[1] = { lumaLaunchPath };

    unmountPartitions();            /* Luma re-initialises the SD card itself */
    prepareArm11ForFirmlaunch();    /* parks the ARM11 core so it can be re-entered */
    chainload(1, argv, firm);       /* runs from ITCM; does not return */
    __builtin_unreachable();
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

    /* Check if sdmc:/loader/tmp/tmp.lock exists; if so, fileDelete succeeds, we delete it and boot Luma */
    if(fileDelete("sdmc:/loader/tmp/tmp.lock"))
    {
        bootLuma();
    }

    /* Keep Luma's boot-key combos (Select = config, Start = chainloader,
       emuNAND/payload hotkeys, ...) working: if anything is held, skip our UI. */
    if(HID_PAD & PIN_BUTTONS) bootLuma();

    showScreen();

    for(;;)
    {
        u32 key = waitInput(false);

        if(key & BUTTON_A)
        {
            static const char payload[] = TARGET_TID ":" TARGET_MEDIA "\n";

            if(!writeFileAtomic(TID_DIR, TID_FILE, payload, sizeof(payload) - 1))
                error("Failed to write " TID_DIR "/" TID_FILE);

            drawString(true, 10, line, COLOR_GREEN, "Written. Booting Luma...");
            bootLuma();
        }
        else if(key & BUTTON_B)
        {
            bootLuma();
        }
    }
}
