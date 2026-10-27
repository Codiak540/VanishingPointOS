#include "titles.h"

static int hexval(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int matchWord(const char *s, size_t avail, const char *word, size_t wl)
{
    if(avail < wl) return 0;
    for(size_t i = 0; i < wl; i++) if(s[i] != word[i]) return 0;
    return 1;
}

const char *titles_media_str(uint8_t media)
{
    switch(media)
    {
        case MEDIA_NAND: return "nand";
        case MEDIA_SD:   return "sd";
        case MEDIA_GAME: return "game";
        default:         return "?";
    }
}

size_t titles_format_request(char *out, size_t outSize, uint64_t tid, uint8_t media)
{
    static const char hex[] = "0123456789ABCDEF";
    const char *m = titles_media_str(media);
    size_t ml = 0;
    while(m[ml]) ml++;

    size_t need = 16 + 1 + ml + 1 + 1;          /* TID ':' media '\n' NUL */
    if(outSize < need) return 0;

    for(int i = 0; i < 16; i++) out[i] = hex[(tid >> (60 - 4 * i)) & 0xF];
    out[16] = ':';
    for(size_t i = 0; i < ml; i++) out[17 + i] = m[i];
    out[17 + ml] = '\n';
    out[18 + ml] = '\0';
    return 18 + ml;
}

int titles_parse(const char *text, size_t len, Title *out, int max)
{
    static const char hex[] = "0123456789ABCDEF";
    int n = 0;
    size_t i = 0;

    while(i < len && n < max)
    {
        size_t start = i;
        while(i < len && text[i] != '\n') i++;
        size_t end = i;
        if(i < len) i++;
        if(end > start && text[end - 1] == '\r') end--;

        const char *l = text + start;
        size_t ll = end - start;

        if(ll == 0 || l[0] == '#' || ll < 19) continue;

        uint64_t tid = 0;
        int ok = 1;
        for(int k = 0; k < 16; k++)
        {
            int v = hexval(l[k]);
            if(v < 0) { ok = 0; break; }
            tid = (tid << 4) | (uint64_t)v;
        }
        if(!ok || l[16] != ':') continue;

        size_t p = 17;
        uint8_t media;
        if(matchWord(l + p, ll - p, "sd", 2))        { media = MEDIA_SD;   p += 2; }
        else if(matchWord(l + p, ll - p, "nand", 4)) { media = MEDIA_NAND; p += 4; }
        else if(matchWord(l + p, ll - p, "game", 4)) { media = MEDIA_GAME; p += 4; }
        else continue;

        Title *t = &out[n];
        t->tid = tid;
        t->media = media;
        size_t nl = 0;

        if(p < ll)
        {
            if(l[p] != ':') continue;               /* garbage after the media word */
            p++;
            while(p < ll && l[p] == ' ') p++;
            while(p < ll && nl < TITLE_NAME_MAX - 1)
            {
                char c = l[p++];
                t->name[nl++] = (c >= 0x20 && c < 0x7F) ? c : '?';
            }
            while(nl > 0 && t->name[nl - 1] == ' ') nl--;
        }

        if(nl == 0)                                  /* no name: show the title ID */
            for(int k = 0; k < 16; k++) t->name[nl++] = hex[(tid >> (60 - 4 * k)) & 0xF];

        t->name[nl] = '\0';
        n++;
    }

    return n;
}
