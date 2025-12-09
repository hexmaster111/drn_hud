#ifndef SLICE_H
#define SLICE_H

typedef struct
{
    char *base;
    size_t len;
} Slice;

#define SLICE_CLIT(CLIT) \
    (Slice) { .base = (CLIT), .len = sizeof(CLIT) }



#endif //SLICE_H