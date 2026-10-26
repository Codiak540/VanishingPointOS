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
*   memcpy adapted from https://github.com/mid-kid/CakesForeveryWan/blob/557a8e8605ab3ee173af6497486e8f22c261d0e2/source/memfuncs.c
*/

/*
 * Optimized it a bit from the original,
 * Instead of copying 1 byte at a time (like it used to), we can check if the memory addresses are 4-byte aligned.
 * If they are, we can copy 4 bytes (a 32-bit word) at a time, and only fall back to byte-copying for any odd remaining bytes at the end.
 */

#include "memory.h"
#include "types.h"

void memcpy(void *dest, const void *src, u32 size)
{
    u8 *d = (u8 *)dest;
    const u8 *s = (const u8 *)src;

    // If pointers are 4-byte aligned and we have enough bytes, copy by 32-bit words
    if ((((u32)d | (u32)s) & 3) == 0 && size >= 4) {
        u32 *d32 = (u32 *)d;
        const u32 *s32 = (const u32 *)s;
        u32 words = size / 4;

        for (u32 i = 0; i < words; i++) {
            d32[i] = s32[i];
        }

        d += words * 4;
        s += words * 4;
        size %= 4;
    }

    // Copy any remaining tail bytes
    while (size--) {
        *d++ = *s++;
    }
}

void memset(void *dest, u32 filler, u32 size)
{
    u8 *d = (u8 *)dest;
    u8 val = (u8)filler;

    // If aligned, replicate the byte across a 32-bit word and fill 4 bytes at a time
    if (((u32)d & 3) == 0 && size >= 4) {
        u32 val32 = val | (val << 8) | (val << 16) | (val << 24);
        u32 *d32 = (u32 *)d;
        u32 words = size / 4;

        for (u32 i = 0; i < words; i++) {
            d32[i] = val32;
        }

        d += words * 4;
        size %= 4;
    }

    // Fill remaining tail bytes
    while (size--) {
        *d++ = val;
    }
}

void memset32(void *dest, u32 filler, u32 size)
{
    u32 *dest32 = (u32 *)dest;
    u32 words = size / 4;

    for(u32 i = 0; i < words; i++)
        dest32[i] = filler;
}
