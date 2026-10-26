//
// Created by george on 10/26/26.
//

#ifndef FIRM_HW_REGISTERS_H
#define FIRM_HW_REGISTERS_H

/*
 * Address documentation:
 *
 *
 * 1. LCD Power & Global Control (0x10202xxx / 0x1014xxxx)
 * These registers control overall display power, master timing blocks, and screen artifact prevention (LCD filling).
 *
 * 0x10141200        Display Subsystem Config        Initialized to 0x1007F during early screen setup.
 * 0x10202014        Master LCD Enable Switch        Set to 0x1 to turn on the LCD controller logic; set to 0 to shut it down.
 * 0x1020200C        LCD Timing Mask Register        Bitwise modified (&= 0xFFFEFFFE) to adjust synchronization masks.
 * 0x10202204        LCD Fill Register (Top)
 * 0x10202A04        LCD Fill Register (Bottom)
 * Set to 0x01000000 to fill the screen buffer lines with solid black (hiding visual garbage/static during boot/firmlaunch transitions), then reset to 0x00000000 when done.
 *
 * 0x10202240        Backlight Brightness (Top)
 * 0x10202A40        Backlight Brightness (Bottom)
 * Assigns the raw brightnessLevel parameter to adjust screen brightness.
 *
 * 0x10202244        Backlight Enable/Config (Top)
 * 0x10202A44        Backlight Enable/Config (Bottom)
 * Set to 0x1023E to power on backlights, or 0 to turn them off.
 *
 *
 * 2. Top Screen Display Controller / PPU (0x104004xx)
 * This memory block manages everything sent to the top physical LCD panel (resolution, geometry, porch timings, stereoscopic 3D framebuffers, and color LUTs).
 *
 * 0x10400400 - 0x1040045C / Display Geometry & Timings
 * Configures horizontal/vertical active lines, front/back porches, and total screen dimensions.
 *
 * 0x10400468        Top-Left Framebuffer (0)
 * 0x1040046C        Top-Left Framebuffer (1)
 * Assigns physical pointers (fbs[0].top_left, fbs[1].top_left) for double-buffered rendering.
 *
 * 0x10400470        Framebuffer Format & Control   Configures pixel format settings (0x80341).
 * 0x10400478        Active Framebuffer Selector    Toggled using the isAlternate flag (u32 1 : 0) to control which buffer is currently displayed.

 * 0x10400480 \
 * 0x10400484        Color Look-Up Table (LUT)      Sets up index 0x10400480 = 0 and loops to write 256 color mapping values into 0x10400484.
 *
 * 0x10400490        Framebuffer Stride (Pitch)     Sets row width spacing (0x2D0 pixels/bytes per row layout).
 *
 * 0x10400494        Top-Right Framebuffer (0)
 * 0x10400498        Top-Right Framebuffer (1)
 * Assigns framebuffers for the right eye (fbs[0].top_right, fbs[1].top_right) to handle stereoscopic 3D depth.
 *
 *
 * 3. Bottom Screen Display Controller / PPU (0x104005xx)
 * Mirrors the top screen controller registers, but mapped specifically for the lower touch-screen display panel.
 *
 * 0x10400500 – 0x1040055C / Bottom Display Geometry & Timings / Configures the touch screen's native resolution and blanking intervals.
 *
 * 0x10400568        Bottom Framebuffer (0)
 * 0x1040056C        Bottom Framebuffer (1)
 * Assigns physical memory pointers (fbs[0].bottom, fbs[1].bottom) for double buffering.
 *
 * 0x10400570        Framebuffer Format & Control   Configures pixel layout format (0x80301).
 * 0x10400578        Active Framebuffer Selector    Selects active buffer state based on isAlternate (u32 1 : 0).
 * 0x10400580 / 0x10400584	Bottom Color LUT        Initializes and populates the bottom screen's 256-entry color palette table.
 * 0x10400590        Framebuffer Stride (Pitch)     Sets row width mapping (0x2D0).
 *
 *
 * 4. GPU Hardware Memory Fill Engine (0x104000xx)
 * Instead of using the CPU to write zeros across the framebuffers slowly, the code offloads this to the GPU's internal hardware fill channels (PSC0 and PSC1).
 *
 * 0x10400010 (REGs_PSC0) & 0x10400020 (REGs_PSC1):
   * [0] = Start memory address (shifted right by 3).
   * [1] = End memory address (shifted right by 3).
   * [2] = Fill pattern value (0 for clearing).
   * [3] = Control command ((2 << 8) | 1 to configure 32-bit mode and trigger the fill action).
   * The while loop checks [3] & 2 to block execution until the hardware fill operation finishes completely.
 *
 *
 * 5. New 3DS Auto-Backlight (ABL) Registers (0x1020225x / 0x102023xx)
 * These are hardware registers unique to the "New" model 3DS revisions for advanced relative-luminance color blending and auto-brightness. On older models, writes to these addresses are ignored safely (Read-As-Zero / Write-Ignored).
 * 0x10202250 / 0x10202254 (and bottom equivalents 0x10202A50 / 0x10202A54): Configuration registers for ABL calibration states.
 * 0x10202300 + 4*i / 0x10202B00 + 4*i: A loop iterating 64 times to zero-fill hardware color-multiplication tables for each group of relative-luminance values.
 *
 *
 * 6. System Execution / Firmlaunch (0x1FFFFCxx)
 * Used right at the end when handing over execution control to a system firmware launch payload.
   * 0x1FFFFC00: Physical destination memory address where memcpy copies the prepareForFirmlaunch binary payload.
   * 0x1FFFFFFC: A control flag cleared to 0 right before executing a function pointer cast directly into address 0x1FFFFC00.
 */

#define REG32(addr) (*(vu32 *)(addr))

// System & GPU Control
#define REG_PDN_GPU_CNT         REG32(0x10141200)
#define REG_MASTER_LCD          REG32(0x10202014)
#define REG_LCD_TIMING_MASK     REG32(0x1020200C)

// LCD Fill Registers
#define REG_LCD_FILL_TOP        REG32(0x10202204)
#define REG_LCD_FILL_BOTTOM     REG32(0x10202A04)

// Backlight Registers
#define REG_BL_BRIGHTNESS_TOP   REG32(0x10202240)
#define REG_BL_BRIGHTNESS_BOT   REG32(0x10202A40)
#define REG_BACKLIGHT_TOP       REG32(0x10202244)
#define REG_BACKLIGHT_BOT       REG32(0x10202A44)

// New 3DS ABL Registers
#define REG_ABL_TOP_250         REG32(0x10202250)
#define REG_ABL_TOP_254         REG32(0x10202254)
#define REG_ABL_BOT_250         REG32(0x10202A50)
#define REG_ABL_BOT_254         REG32(0x10202A54)
#define REG_ABL_BLEND_TOP(i)    REG32(0x10202300 + 4*(i))
#define REG_ABL_BLEND_BOT(i)    REG32(0x10202B00 + 4*(i))

// PPU Base & Offset Helpers
#define REG_TOP_PPU(offset)     REG32(0x10400400 + (offset))
#define REG_BOT_PPU(offset)     REG32(0x10400500 + (offset))

// PPU Register Offsets
#define PPU_OFF_TIMING_00       0x00
#define PPU_OFF_TIMING_04       0x04
#define PPU_OFF_TIMING_08       0x08
#define PPU_OFF_TIMING_0C       0x0C
#define PPU_OFF_TIMING_10       0x10
#define PPU_OFF_TIMING_14       0x14
#define PPU_OFF_TIMING_18       0x18
#define PPU_OFF_TIMING_1C       0x1C
#define PPU_OFF_TIMING_20       0x20
#define PPU_OFF_TIMING_24       0x24
#define PPU_OFF_TIMING_28       0x28
#define PPU_OFF_TIMING_2C       0x2C
#define PPU_OFF_TIMING_30       0x30
#define PPU_OFF_TIMING_34       0x34
#define PPU_OFF_TIMING_38       0x38
#define PPU_OFF_TIMING_3C       0x3C
#define PPU_OFF_TIMING_40       0x40
#define PPU_OFF_TIMING_44       0x44
#define PPU_OFF_TIMING_48       0x48
#define PPU_OFF_CONFIG_5C       0x5C
#define PPU_OFF_CONFIG_60       0x60
#define PPU_OFF_CONFIG_64       0x64
#define PPU_OFF_CONFIG_74       0x74
#define PPU_OFF_CONFIG_9C       0x9C

// Explicit Top Screen PPU Aliases
#define REG_TOP_FB_LEFT_0       REG_TOP_PPU(0x68)
#define REG_TOP_FB_LEFT_1       REG_TOP_PPU(0x6C)
#define REG_TOP_FB_FORMAT       REG_TOP_PPU(0x70)
#define REG_TOP_FB_SELECT       REG_TOP_PPU(0x78)
#define REG_TOP_LUT_CNT         REG_TOP_PPU(0x80)
#define REG_TOP_LUT_DATA        REG_TOP_PPU(0x84)
#define REG_TOP_STRIDE          REG_TOP_PPU(0x90)
#define REG_TOP_FB_RIGHT_0      REG_TOP_PPU(0x94)
#define REG_TOP_FB_RIGHT_1      REG_TOP_PPU(0x98)

// Explicit Bottom Screen PPU Aliases
#define REG_BOT_FB_0            REG_BOT_PPU(0x68)
#define REG_BOT_FB_1            REG_BOT_PPU(0x6C)
#define REG_BOT_FB_FORMAT       REG_BOT_PPU(0x70)
#define REG_BOT_FB_SELECT       REG_BOT_PPU(0x78)
#define REG_BOT_LUT_CNT         REG_BOT_PPU(0x80)
#define REG_BOT_LUT_DATA        REG_BOT_PPU(0x84)
#define REG_BOT_STRIDE          REG_BOT_PPU(0x90)

// System & Global Constants
#define DISP_SUB_CONFIG_INIT    0x1007F
#define LCD_FILL_BLACK          0x01000000
#define LCD_FILL_UNSET          0x00000000
#define LCD_TIME_SYNC_MASK      0xFFFEFFFE
#define BL_POWER                0x1023E
#define MASTER_LCD_ENABLE       0x00000001
#define MASTER_LCD_DISABLE      0x00000000

#define LCD_FORMAT_TOP          0x80341
#define LCD_FORMAT_BOT          0x80301
#define PPU_BURST_CONFIG        0x00010501
#define LCD_STRIDE_VALUE        0x000002D0

// Top Screen PPU Geometry & Timing Values
#define TOP_TIMING_00           0x000001c2
#define TOP_TIMING_04           0x000000d1
#define TOP_TIMING_08           0x000001c1
#define TOP_TIMING_0C           0x000001c1
#define TOP_TIMING_10           0x00000000
#define TOP_TIMING_14           0x000000cf
#define TOP_TIMING_18           0x000000d1
#define TOP_TIMING_1C           0x01c501c1
#define TOP_TIMING_20           0x00010000
#define TOP_TIMING_24           0x0000019d
#define TOP_TIMING_28           0x00000002
#define TOP_TIMING_2C           0x00000192
#define TOP_TIMING_30           0x00000192
#define TOP_TIMING_34           0x00000192
#define TOP_TIMING_38           0x00000001
#define TOP_TIMING_3C           0x00000002
#define TOP_TIMING_40           0x01960192
#define TOP_TIMING_44           0x00000000
#define TOP_TIMING_48           0x00000000
#define TOP_CFG_5C              0x00f00190  // Width 400
#define TOP_CFG_60              0x01c100d1
#define TOP_CFG_64              0x01920002

// Bottom Screen PPU Geometry & Timing Values
#define BOT_TIMING_00           0x000001c2
#define BOT_TIMING_04           0x000000d1
#define BOT_TIMING_08           0x000001c1
#define BOT_TIMING_0C           0x000001c1
#define BOT_TIMING_10           0x000000cd
#define BOT_TIMING_14           0x000000cf
#define BOT_TIMING_18           0x000000d1
#define BOT_TIMING_1C           0x01c501c1
#define BOT_TIMING_20           0x00010000
#define BOT_TIMING_24           0x0000019d
#define BOT_TIMING_28           0x00000052
#define BOT_TIMING_2C           0x00000192
#define BOT_TIMING_30           0x00000192
#define BOT_TIMING_34           0x0000004f
#define BOT_TIMING_38           0x00000050
#define BOT_TIMING_3C           0x00000052
#define BOT_TIMING_40           0x01980194
#define BOT_TIMING_44           0x00000000
#define BOT_TIMING_48           0x00000011
#define BOT_CFG_5C              0x00f00140  // Width 320
#define BOT_CFG_60              0x01c100d1
#define BOT_CFG_64              0x01920052

#endif //FIRM_HW_REGISTERS_H
