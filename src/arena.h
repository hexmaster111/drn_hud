#ifndef ARENA_H
#define ARENA_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Arena
{
    void *data;
    size_t size, bump;
    struct Arena *next;
} Arena;

Arena ArenaNew(size_t cap);
void *ArenaMalloc(Arena *a, size_t sz);
void ArenaFree(Arena *a);
char *LoadFileTextArena(Arena *ar, const char *fpath, size_t *len);
char *StringCopyArena(Arena *a, const char *from);

#endif // ARENA_H

#ifdef ARENA_IMPL
#undef ARENA_IMPL

char *StringCopyArena(Arena *a, const char *from)
{
    size_t sz = strlen(from) + 1;
    char *ret = ArenaMalloc(a, sz);
    strcpy(ret, from);
    return ret;
}


// loads file text, allocating it into the arena
char *LoadFileTextArena(Arena *ar, const char *fpath, size_t *len)
{
    char *text = NULL;

    FILE *file = fopen(fpath, "rt");
    if (file == 0)
        return 0;

    fseek(file, 0, SEEK_END);
    unsigned int size = (unsigned int)ftell(file);
    fseek(file, 0, SEEK_SET);

    if (0 >= size)
        return 0;

    text = ArenaMalloc(ar, (size + 1) * sizeof(char));

    unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

    if (count < size)
    {
        printf("really?! this happends?????\n");
        abort();
    }

    text[count] = '\0';

    if (len)
        *len = size;

    fclose(file);

    return text;
}



Arena ArenaNew(size_t cap)
{
    Arena ret = {0};

    ret.data = malloc(cap);
    memset(ret.data, 0, cap);

    ret.size = cap;
    ret.bump = 0;
    ret.next = 0;

    return ret;
}

void *ArenaMalloc(Arena *a, size_t sz)
{
    Arena *cur = a;

    if (sz > a->size)
        return 0;

    while (cur->next != 0)
        cur = cur->next;

    if (cur->bump + sz > cur->size)
    {
        // we need to alloc a new arena
        Arena new = ArenaNew(a->size);
        Arena *nextPtr = malloc(sizeof(Arena));
        *nextPtr = new;
        cur->next = nextPtr;
        cur = cur->next;
    }

    void *ret = cur->data + cur->bump;
    cur->bump += sz;

    return ret;
}

void ArenaFree(Arena *a)
{
    /*
    Arena -> next -> next -> next
    */

    if (a->next)
    {
        ArenaFree(a->next); // cralw down to the bottem
    }

    if (a->next)
    {
        free(a->next);
        a->next = 0;
    }

    if (a->data)
    {
        free(a->data);
        a->data = 0;
    }

    a->bump = 0;
    a->size = 0;
}

#endif // ARENA_IMPL

#ifdef ARENA_TEST
#undef ARENA_TEST

int main()
{
    Arena n = ArenaNew(16);

    int *n0 = ArenaMalloc(&n, sizeof(int));
    *n0 = -1;

    int *n1 = ArenaMalloc(&n, sizeof(int));
    int *n2 = ArenaMalloc(&n, sizeof(int));
    int *n3 = ArenaMalloc(&n, sizeof(int));
    int *n4 = ArenaMalloc(&n, sizeof(int));
    int *n5 = ArenaMalloc(&n, sizeof(int));

    ArenaFree(&n);

    return 0;
}
#endif // ARENA_TEST
