#pragma once

/* Real Luma3DS boot.firm, moved out of the SD root so this FIRM can be /boot.firm. */
#define LUMA_FIRM_PATH   "sdmc:/loader/luma.firm"

/* Where the companion app (running as Luma's autoboot 3DSX) looks for the TID. */
#define TID_DIR          "sdmc:/loader/tmp"
#define TID_FILE         "tid.tmp"

/* Installed-title list: written by the companion's scan, read by the menu below (plain
   text, see titles.h; you can also edit it by hand). */
#define TITLES_FILE      "sdmc:/loader/titles.txt"

/* Optional theme images (made with tools/png2fb.py): top.bin, bottom.bin, logo.spr */
#define THEME_DIR        "sdmc:/loader/theme"

/* Written by the menu's "rescan" key; makes the companion rebuild TITLES_FILE. */
#define SCAN_REQ_FILE    "scan.req"

/* Written by the companion right before it asks NS to reboot into a title. That reboot
   comes back through this FIRM; seeing (and deleting) the lock makes us skip the menu
   and hand straight to Luma. */
#define LOCK_FILE        "sdmc:/loader/tmp/tmp.lock"

/* Alternative to the lock file: 1 = also skip the menu whenever we are started by a
   firmlaunch (a reboot, as opposed to a cold boot). Luma tells the two apart by the
   magic value passed to main(): 0xBEEF = cold boot via boot9strap, 0xBABE = firmlaunch.
   Off by default; untested on hardware. */
#define AUTO_HANDOFF_ON_FIRMLAUNCH 0

/* Fallback menu entry, shown only when TITLES_FILE is missing, empty or unreadable.
   Set it to the title ID of an installed application (FBI's title list shows it).
   TARGET_MEDIA is one of: sd, nand, game */
#define TARGET_TID       "0004000000000000"
#define TARGET_MEDIA     "sd"

/* 1 = before the handoff, print the FIRM's size/entry points/sections and wait
   for A. Useful for telling "handoff never happened" from "Luma ran and crashed". */
#define DEBUG_STEPS      0
