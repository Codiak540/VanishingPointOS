/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2020 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

/*
*   Screen init code by dark_samus, bil1s, Normmatt, delebile and others
*   LCD deinit code by tiniVi
*/

#include "types.h"
#include "memory.h"
#include "hw_registers.h"


void prepareForFirmlaunch(void);
extern u32 prepareForFirmlaunchSize;
extern volatile Arm11Operation operation;

static void initScreens(u32 brightnessLevel, struct fb *fbs)
{
    REG_PDN_GPU_CNT = DISP_SUB_CONFIG_INIT;
    REG_LCD_FILL_TOP = LCD_FILL_BLACK; // Hide potential boot garbage
    REG_LCD_FILL_BOTTOM = LCD_FILL_BLACK;

    REG_MASTER_LCD = MASTER_LCD_ENABLE;
    REG_LCD_TIMING_MASK &= LCD_TIME_SYNC_MASK;
    REG_BL_BRIGHTNESS_TOP = brightnessLevel;
    REG_BL_BRIGHTNESS_BOT = brightnessLevel;
    REG_BACKLIGHT_TOP = BL_POWER;
    REG_BACKLIGHT_BOT = BL_POWER;

    // Top screen geometry & timings
    REG_TOP_PPU(PPU_OFF_TIMING_00) = TOP_TIMING_00;
    REG_TOP_PPU(PPU_OFF_TIMING_04) = TOP_TIMING_04;
    REG_TOP_PPU(PPU_OFF_TIMING_08) = TOP_TIMING_08;
    REG_TOP_PPU(PPU_OFF_TIMING_0C) = TOP_TIMING_0C;
    REG_TOP_PPU(PPU_OFF_TIMING_10) = TOP_TIMING_10;
    REG_TOP_PPU(PPU_OFF_TIMING_14) = TOP_TIMING_14;
    REG_TOP_PPU(PPU_OFF_TIMING_18) = TOP_TIMING_18;
    REG_TOP_PPU(PPU_OFF_TIMING_1C) = TOP_TIMING_1C;
    REG_TOP_PPU(PPU_OFF_TIMING_20) = TOP_TIMING_20;
    REG_TOP_PPU(PPU_OFF_TIMING_24) = TOP_TIMING_24;
    REG_TOP_PPU(PPU_OFF_TIMING_28) = TOP_TIMING_28;
    REG_TOP_PPU(PPU_OFF_TIMING_2C) = TOP_TIMING_2C;
    REG_TOP_PPU(PPU_OFF_TIMING_30) = TOP_TIMING_30;
    REG_TOP_PPU(PPU_OFF_TIMING_34) = TOP_TIMING_34;
    REG_TOP_PPU(PPU_OFF_TIMING_38) = TOP_TIMING_38;
    REG_TOP_PPU(PPU_OFF_TIMING_3C) = TOP_TIMING_3C;
    REG_TOP_PPU(PPU_OFF_TIMING_40) = TOP_TIMING_40;
    REG_TOP_PPU(PPU_OFF_TIMING_44) = TOP_TIMING_44;
    REG_TOP_PPU(PPU_OFF_TIMING_48) = TOP_TIMING_48;
    REG_TOP_PPU(PPU_OFF_CONFIG_5C) = TOP_CFG_5C;
    REG_TOP_PPU(PPU_OFF_CONFIG_60) = TOP_CFG_60;
    REG_TOP_PPU(PPU_OFF_CONFIG_64) = TOP_CFG_64;

    REG_TOP_FB_LEFT_0   = (u32)fbs[0].top_left;
    REG_TOP_FB_LEFT_1   = (u32)fbs[1].top_left;
    REG_TOP_FB_FORMAT   = LCD_FORMAT_TOP;
    REG_TOP_PPU(PPU_OFF_CONFIG_74) = PPU_BURST_CONFIG;
    REG_TOP_FB_SELECT   = 0;
    REG_TOP_FB_RIGHT_0  = (u32)fbs[0].top_right;
    REG_TOP_FB_RIGHT_1  = (u32)fbs[1].top_right;
    REG_TOP_STRIDE      = LCD_STRIDE_VALUE;
    REG_TOP_PPU(PPU_OFF_CONFIG_9C) = 0x00000000;

    // Top Color LUT
    REG_TOP_LUT_CNT = 0;
    for(u32 i = 0; i < 256; i++)
        REG_TOP_LUT_DATA = 0x10101 * i;

    // Bottom screen geometry & timings
    REG_BOT_PPU(PPU_OFF_TIMING_00) = BOT_TIMING_00;
    REG_BOT_PPU(PPU_OFF_TIMING_04) = BOT_TIMING_04;
    REG_BOT_PPU(PPU_OFF_TIMING_08) = BOT_TIMING_08;
    REG_BOT_PPU(PPU_OFF_TIMING_0C) = BOT_TIMING_0C;
    REG_BOT_PPU(PPU_OFF_TIMING_10) = BOT_TIMING_10;
    REG_BOT_PPU(PPU_OFF_TIMING_14) = BOT_TIMING_14;
    REG_BOT_PPU(PPU_OFF_TIMING_18) = BOT_TIMING_18;
    REG_BOT_PPU(PPU_OFF_TIMING_1C) = BOT_TIMING_1C;
    REG_BOT_PPU(PPU_OFF_TIMING_20) = BOT_TIMING_20;
    REG_BOT_PPU(PPU_OFF_TIMING_24) = BOT_TIMING_24;
    REG_BOT_PPU(PPU_OFF_TIMING_28) = BOT_TIMING_28;
    REG_BOT_PPU(PPU_OFF_TIMING_2C) = BOT_TIMING_2C;
    REG_BOT_PPU(PPU_OFF_TIMING_30) = BOT_TIMING_30;
    REG_BOT_PPU(PPU_OFF_TIMING_34) = BOT_TIMING_34;
    REG_BOT_PPU(PPU_OFF_TIMING_38) = BOT_TIMING_38;
    REG_BOT_PPU(PPU_OFF_TIMING_3C) = BOT_TIMING_3C;
    REG_BOT_PPU(PPU_OFF_TIMING_40) = BOT_TIMING_40;
    REG_BOT_PPU(PPU_OFF_TIMING_44) = BOT_TIMING_44;
    REG_BOT_PPU(PPU_OFF_TIMING_48) = BOT_TIMING_48;
    REG_BOT_PPU(PPU_OFF_CONFIG_5C) = BOT_CFG_5C;
    REG_BOT_PPU(PPU_OFF_CONFIG_60) = BOT_CFG_60;
    REG_BOT_PPU(PPU_OFF_CONFIG_64) = BOT_CFG_64;

    REG_BOT_FB_0        = (u32)fbs[0].bottom;
    REG_BOT_FB_1        = (u32)fbs[1].bottom;
    REG_BOT_FB_FORMAT   = LCD_FORMAT_BOT;
    REG_BOT_PPU(PPU_OFF_CONFIG_74) = PPU_BURST_CONFIG;
    REG_BOT_FB_SELECT   = 0;
    REG_BOT_STRIDE      = LCD_STRIDE_VALUE;
    REG_BOT_PPU(PPU_OFF_CONFIG_9C) = 0x00000000;

    // Bottom Color LUT
    REG_BOT_LUT_CNT = 0;
    for(u32 i = 0; i < 256; i++)
        REG_BOT_LUT_DATA = 0x10101 * i;

    REG_LCD_FILL_TOP = LCD_FILL_UNSET;
    REG_LCD_FILL_BOTTOM = LCD_FILL_UNSET;
}

static void setupFramebuffers(struct fb *fbs)
{
    REG_LCD_FILL_TOP = LCD_FILL_BLACK;
    REG_LCD_FILL_BOTTOM = LCD_FILL_BLACK;

    REG_TOP_FB_LEFT_0  = (u32)fbs[0].top_left;
    REG_TOP_FB_LEFT_1  = (u32)fbs[1].top_left;
    REG_TOP_FB_RIGHT_0 = (u32)fbs[0].top_right;
    REG_TOP_FB_RIGHT_1 = (u32)fbs[1].top_right;
    REG_BOT_FB_0       = (u32)fbs[0].bottom;
    REG_BOT_FB_1       = (u32)fbs[1].bottom;

    REG_TOP_FB_FORMAT  = LCD_FORMAT_TOP;
    REG_TOP_FB_SELECT  = 0;
    REG_TOP_STRIDE     = LCD_STRIDE_VALUE;
    REG_BOT_FB_FORMAT  = LCD_FORMAT_BOT;
    REG_BOT_FB_SELECT  = 0;
    REG_BOT_STRIDE     = LCD_STRIDE_VALUE;

    REG_LCD_FILL_TOP = LCD_FILL_UNSET;
    REG_LCD_FILL_BOTTOM = LCD_FILL_UNSET;
}

static void clearScreens(struct fb *fb)
{
    vu32 *REGs_PSC0 = (vu32 *)0x10400010,
         *REGs_PSC1 = (vu32 *)0x10400020;

    REGs_PSC0[0] = (u32)fb->top_left >> 3;
    REGs_PSC0[1] = (u32)(fb->top_left + SCREEN_TOP_FBSIZE) >> 3;
    REGs_PSC0[2] = 0;
    REGs_PSC0[3] = (2 << 8) | 1;

    REGs_PSC1[0] = (u32)fb->bottom >> 3;
    REGs_PSC1[1] = (u32)(fb->bottom + SCREEN_BOTTOM_FBSIZE) >> 3;
    REGs_PSC1[2] = 0;
    REGs_PSC1[3] = (2 << 8) | 1;

    while(!((REGs_PSC0[3] & 2) && (REGs_PSC1[3] & 2)));
}

static void swapFramebuffers(bool isAlternate)
{
    u32 isAlternateTmp = isAlternate ? 1 : 0;
    REG_TOP_FB_SELECT = (REG_TOP_FB_SELECT & 0xFFFFFFFE) | isAlternateTmp;
    REG_BOT_FB_SELECT = (REG_BOT_FB_SELECT & 0xFFFFFFFE) | isAlternateTmp;
}

static void updateBrightness(u32 brightnessLevel)
{
    REG_BL_BRIGHTNESS_TOP = brightnessLevel;
    REG_BL_BRIGHTNESS_BOT = brightnessLevel;
}

static void deinitScreens(void)
{
    REG_BACKLIGHT_BOT = 0;
    REG_BACKLIGHT_TOP = 0;
    REG_MASTER_LCD = MASTER_LCD_DISABLE;
}

static void zerofillN3dsAblRegisters(void)
{
    REG_ABL_TOP_250 = 0;
    REG_ABL_TOP_254 = 0;
    REG_ABL_BOT_250 = 0;
    REG_ABL_BOT_254 = 0;

    for (u32 i = 0; i < 64; i++) {
        REG_ABL_BLEND_TOP(i) = 0;
        REG_ABL_BLEND_BOT(i) = 0;
    }
}

void main(void)
{
    operation = ARM11_READY;

    while(true)
    {
        switch(operation)
        {
            case ARM11_READY:
                continue;
            case INIT_SCREENS:
                initScreens(*(vu32 *)ARM11_PARAMETERS_ADDRESS, (struct fb *)(ARM11_PARAMETERS_ADDRESS + 4));
                break;
            case SETUP_FRAMEBUFFERS:
                setupFramebuffers((struct fb *)ARM11_PARAMETERS_ADDRESS);
                break;
            case CLEAR_SCREENS:
                clearScreens((struct fb *)ARM11_PARAMETERS_ADDRESS);
                break;
            case SWAP_FRAMEBUFFERS:
                swapFramebuffers(*(volatile bool *)ARM11_PARAMETERS_ADDRESS);
                break;
            case UPDATE_BRIGHTNESS:
                updateBrightness(*(vu32 *)ARM11_PARAMETERS_ADDRESS);
                break;
            case DEINIT_SCREENS:
                deinitScreens();
                break;
            case ZEROFILL_N3DS_ABL_REGISTERS:
                zerofillN3dsAblRegisters();
                break;
            case PREPARE_ARM11_FOR_FIRMLAUNCH:
                memcpy((void *)0x1FFFFC00, (void *)prepareForFirmlaunch, prepareForFirmlaunchSize);
                *(vu32 *)0x1FFFFFFC = 0;
                ((void (*)(u32, volatile Arm11Operation *))0x1FFFFC00)(ARM11_READY, &operation);
        }

        operation = ARM11_READY;
    }
}