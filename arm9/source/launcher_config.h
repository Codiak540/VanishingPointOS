#pragma once

/* Real Luma3DS boot.firm, moved out of the SD root so this FIRM can be /boot.firm. */
#define LUMA_FIRM_PATH   "sdmc:/loader/luma.firm"

/* Where the companion app (running as Luma's autoboot 3DSX) looks for the TID. */
#define TID_DIR          "sdmc:/loader/tmp"
#define TID_FILE         "tid.tmp"

/* PoC hardcoded launch target. Replace TARGET_TID with the title ID of an
   application that is installed (FBI's title list shows it).
   TARGET_MEDIA is one of: sd, nand, game */
#define TARGET_TID       "0004000000033500"
#define TARGET_MEDIA     "sd"
