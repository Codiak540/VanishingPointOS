/* SD-only filesystem helpers on top of Luma's FatFS port (GPLv3, see LICENSE). */
#include "fs.h"
#include "fatfs/ff.h"
#include "memory.h"

static FATFS sdFs;
static bool sdMounted = false;

bool mountSd(void)
{
    if(!sdMounted)
        sdMounted = f_mount(&sdFs, "sdmc:", 1) == FR_OK;
    return sdMounted && f_chdrive("sdmc:") == FR_OK;
}

void unmountPartitions(void)
{
    if(sdMounted) f_unmount("sdmc:");
    sdMounted = false;
}

/* Same behaviour as Luma's fileRead. */
u32 fileRead(void *dest, const char *path, u32 maxSize)
{
    FIL file;
    FRESULT result = FR_OK;
    u32 ret = 0;

    if(f_open(&file, path, FA_READ) != FR_OK) return 0;

    u32 size = f_size(&file);
    if(dest == NULL) ret = size;
    else if(size <= maxSize)
        result = f_read(&file, dest, size, (unsigned int *)&ret);
    result |= f_close(&file);

    return result == FR_OK ? ret : 0;
}

/* Creates every directory in `dir` ("sdmc:/a/b"), ignoring ones that exist. */
static bool makeDirs(const char *dir)
{
    char buf[128];
    u32 n = strlen(dir);
    if(n >= sizeof(buf)) return false;
    memcpy(buf, dir, n + 1);

    for(u32 i = 6; i <= n; i++) /* skip "sdmc:/" */
    {
        if(buf[i] == '/' || buf[i] == '\0')
        {
            char c = buf[i];
            buf[i] = '\0';
            FRESULT r = f_mkdir(buf);
            if(r != FR_OK && r != FR_EXIST) return false;
            buf[i] = c;
        }
    }
    return true;
}

/* Writes dir/name by writing dir/name.new and renaming it into place, so a
   power cut can't leave a half-written file under the final name. */
bool writeFileAtomic(const char *dir, const char *name, const void *data, u32 len)
{
    char fin[160], tmp[168];
    u32 dl = strlen(dir), nl = strlen(name);
    if(dl + nl + 6 >= sizeof(fin)) return false;

    memcpy(fin, dir, dl);
    fin[dl] = '/';
    memcpy(fin + dl + 1, name, nl + 1);
    memcpy(tmp, fin, dl + 1 + nl);
    memcpy(tmp + dl + 1 + nl, ".new", 5);

    if(!makeDirs(dir)) return false;

    FIL f;
    unsigned int bw = 0;
    if(f_open(&f, tmp, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return false;
    FRESULT r = f_write(&f, data, len, &bw);
    if(r == FR_OK) r = f_sync(&f);
    r |= f_close(&f);
    if(r != FR_OK || bw != len) { f_unlink(tmp); return false; }

    f_unlink(fin);                       /* f_rename fails if the target exists */
    return f_rename(tmp, fin) == FR_OK;
}
bool fileConsume(const char *path)
{
    return f_unlink(path) == FR_OK;
}