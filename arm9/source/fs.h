#pragma once
#include "types.h"

bool mountSd(void);
void unmountPartitions(void);   /* also called by Luma's utils.c (mcuPowerOff) */
u32  fileRead(void *dest, const char *path, u32 maxSize);
bool writeFileAtomic(const char *dir, const char *name, const void *data, u32 len);
bool fileConsume(const char *path);   /* true iff the file existed and was deleted */
