#pragma once
/* Title-list file format shared with the companion (portable C, no Luma dependencies):
 *
 *   # comment
 *   0004000000033500:sd:The Legend of Zelda: Ocarina of Time 3D
 *   0004000000123400:game
 *
 * i.e. <16 hex digits>:<sd|nand|game>[:<name>]. Lines that don't parse are skipped.
 * Names are reduced to printable ASCII (the FIRM's font) and cut to TITLE_NAME_MAX-1.
 * A missing name falls back to the title ID. The file is hand-editable.
 */
#include <stdint.h>
#include <stddef.h>

#define TITLE_NAME_MAX 48
#define TITLES_MAX     256

enum { MEDIA_NAND = 0, MEDIA_SD = 1, MEDIA_GAME = 2 };   /* same values as FS_MediaType */

typedef struct
{
    uint64_t tid;
    uint8_t  media;
    char     name[TITLE_NAME_MAX];
} Title;

/* Parses up to `max` titles from `text`. Returns the number parsed. */
int titles_parse(const char *text, size_t len, Title *out, int max);

const char *titles_media_str(uint8_t media);

/* Writes "<16 hex>:<media>\n" (the companion's tid.tmp format). Returns the length
   written, or 0 if `outSize` is too small. */
size_t titles_format_request(char *out, size_t outSize, uint64_t tid, uint8_t media);
