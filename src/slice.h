#ifndef SLICE_H
#define SLICE_H

typedef struct Slice
{
    char *base;
    size_t len;
} Slice;

#define SLICE_CLIT(CLIT) \
    (Slice) { .base = (CLIT), .len = sizeof(CLIT) }

Slice Slice_CStr(char *clit);

#endif // SLICE_H

#ifdef SLICE_IMPL
#undef SLICE_IMPL
Slice Slice_CStr(char *clit)
{
    Slice s = {0};
    s.base = clit;
    s.len = strlen(clit);
    return s;
}
#endif // SLICE_IMPL