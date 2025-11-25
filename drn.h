
// drn.h
#ifndef DRN_H
#define DRN_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "arena.h"

typedef struct
{
    char *base;
    size_t len;
} Slice;

#define SLICE_CLIT(CLIT) \
    (Slice) { .base = (CLIT), .len = sizeof(CLIT) }

typedef struct
{
    Slice *items;
    size_t cap, top;
} SliceStack, SliceList;

void SliceStack_Push(SliceStack *, Slice);
void SliceStack_Clear(SliceStack *);
Slice SliceStack_Pop(SliceStack *);

typedef enum
{
    DNK_TASK,
    DNK_CONDITION, // has inner

    LEX_END_OF_FILE,
    LEX_ERROR,
    LEX_OK,
} DrnNodeKind;

struct DrnNode
{
    DrnNodeKind kind;
    Slice code;

    struct DrnNode *next;
    struct DrnNode *inner;
};
typedef struct DrnNode DrnNode;

typedef struct
{
    Arena arena;
    DrnNode *start;
} DrnScript;

typedef struct DrnLoadRes
{
    DrnScript script;
    bool isOk;   // 1 = ok , 0 = error;
    char *error; // user must free me
} DrnLoadRes;

DrnLoadRes LoadDrnScriptFromFile(const char *path);

typedef struct
{
    SliceStack task_stack;
} DrnExec;

#endif // DRN_H

// drn.c
#ifdef DRN_IMPL
#undef DRN_IMPL

char *StringCopy(const char *from)
{
    size_t sz = strlen(from) + 1;
    char *ret = malloc(sz);
    memset(ret, 0, sz);
    strcpy(ret, from);
    return ret;
}

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

struct DrnLexer
{
    char *end;
    char *now;

    char *filename;
    size_t line;
};

struct DrnToken
{
    DrnNodeKind kind;
    Slice code;

    int indent; // 0 = root

    struct
    {
        int line;
    } error_pos;
};

#define MIN(A, B) (A) > (B) ? (B) : (A)

Slice DrnLex_ReadToNewline(struct DrnLexer *s)
{
    Slice ret = {.base = s->now, .len = 0};

    while (s->end > s->now && s->now[0] != '\n')
    {
        s->now += 1;
    }

    ret.len = s->now - ret.base;

    // check it: we are removing the new line here
    if (s->now[0] == '\n')
        s->now += 1; // skip new lines

    s->line += 1;

    return ret;
}

void DrnLex_GetAndStripIdent(Slice *s, int *out_indent)
{
    *out_indent = 0;
    while (memcmp(s->base, "  ", 2) == 0)
    {
        s->base += 2;
        s->len -= 2;

        *out_indent += 1;
    }
}

struct DrnToken DrnLex_Next(struct DrnLexer *s)
{
    // TASK: words words words\n
    // CONDITION: words words words\n

#define TASK_STR "TASK:"
#define COND_STR "CONDITION:"

SKIP:

    int indent;

    Slice line = DrnLex_ReadToNewline(s);
    DrnLex_GetAndStripIdent(&line, &indent);

    if (line.len == 0 && s->now == s->end)
    {
        return (struct DrnToken){.kind = LEX_END_OF_FILE};
    }
    else if (line.base[0] == '#' || line.base[0] == '\n')
    {
        // empty line, comment, newline
        // comment line, skip DRONES NEVER PARSE THE COMMENTS!
        goto SKIP;
    }
    else if (memcmp(line.base, TASK_STR, sizeof(TASK_STR) - 1) == 0)
    {
        return (struct DrnToken){.kind = DNK_TASK, .code = line, .indent = indent};
    }
    else if (memcmp(line.base, COND_STR, sizeof(COND_STR) - 1) == 0)
    {
        return (struct DrnToken){.kind = DNK_CONDITION, .code = line, .indent = indent};
    }

    return (struct DrnToken){.kind = LEX_ERROR, .error_pos = {.line = s->line}};

#undef TASK_STR
#undef COND_STR
}

struct DrnToken DrnLex_Peek(struct DrnLexer *s)
{
    // ineficnet af, we re-do our work ever peek, but, this is fast to impl
    struct DrnLexer bck = *s;
    struct DrnToken peek = DrnLex_Next(s);
    *s = bck;
    return peek;
}

void DrnPrint(FILE *f, DrnScript s)
{

    DrnNode *n = s.start;

    while (n)
    {
        fprintf(f, "%.*s\n", n->code.len, n->code.base);
        n = n->next;
    }
}

#define TODO(MSG)                                                   \
    do                                                              \
    {                                                               \
        printf("%s:%d TODO: %s\n", __FILE_NAME__, __LINE__, (MSG)); \
        abort();                                                    \
    } while (0);

// prototype
DrnNode *Parse(struct DrnLexer *lxr, Arena *arena);

DrnNode *ParseTask(struct DrnLexer *lxr, Arena *arena)
{
    struct DrnToken tk = DrnLex_Next(lxr);
    assert(tk.kind == DNK_TASK && "ParseTask() on a non task node?");
    DrnNode *n = ArenaMalloc(arena, sizeof(DrnNode));
    n->code = tk.code;
    n->kind = DNK_TASK;
    n->next = Parse(lxr, arena);
    return n;
}

DrnNode *ParseCondition(struct DrnLexer *lxr, Arena *arena)
{
    struct DrnToken tk = DrnLex_Next(lxr);
    assert(tk.kind == DNK_CONDITION && "ParseCondition() on a non cond node?");
    DrnNode *n = ArenaMalloc(arena, sizeof(DrnNode));
    n->kind = DNK_CONDITION;
    n->code = tk.code;

    int cond_depth = tk.indent;
    int body_depth = tk.indent + 1;

    struct DrnToken pk

    return n;
}

DrnNode *Parse(struct DrnLexer *lxr, Arena *arena)
{
    struct DrnToken peek = DrnLex_Peek(lxr);

    switch (peek.kind)
    {
    case DNK_CONDITION:
        return ParseCondition(lxr, arena);
    case DNK_TASK:
        return ParseTask(lxr, arena);
    default:
        TODO("UNKNOWN TOKEN KIND");
        return 0;
    }
}

int ParseFile(struct DrnLexer *lxr, DrnScript *scr)
{
    scr->start = Parse(lxr, &scr->arena);

    if (scr->start == 0)
        return LEX_ERROR;

    return LEX_OK;
}

DrnLoadRes LoadDrnScriptFromFile(const char *path)
{
    if (!FileExists(path))
    {
        return (DrnLoadRes){
            .error = StringCopy(TextFormat("ERROR: \'%s\' Not Found", path)),
            .isOk = 0,
            .script = {0},
        };
    }

    DrnScript scr = {0};
    scr.arena = ArenaNew(1 << 16);
    size_t filetextlen = 0;
    char *filetext = LoadFileTextArena(&scr.arena, path, &filetextlen);

    struct DrnLexer lxr = {
        .filename = StringCopyArena(&scr.arena, path),
        .now = filetext,
        .end = filetext + filetextlen,
        .line = 0,
    };

    int res = ParseFile(&lxr, &scr);

    if (res == LEX_ERROR)
    {
        ArenaFree(&scr.arena);
        return (DrnLoadRes){
            .error = StringCopy(TextFormat("Syntax Error: %s:%d\n", path, lxr.line)),
            .isOk = 0,
            .script = {0},
        };
    }

    DrnPrint(stdout, scr);

    return (DrnLoadRes){
        .error = 0,
        .isOk = 1,
        .script = scr,
    };
}

Slice SliceStack_Pop(SliceStack *stack)
{
    if (!stack->items)
        return (Slice){0};
    if (0 >= stack->top)
        return (Slice){0};

    stack->top -= 1;
    Slice s = stack->items[stack->top];

    return s;
}

void SliceStack_Push(SliceStack *stack, Slice item)
{
    if (!stack->items)
    {
        // new stack, allocate array
        const int inital_cap = 5;
        const size_t sz = sizeof(Slice) * inital_cap;
        stack->items = malloc(sz);
        memset(stack->items, 0, sz);
        stack->cap = inital_cap;
        stack->top = 0;
    }

    if (stack->top + 1 > stack->cap)
    {
        // grow array

        size_t newcap = stack->cap * 2;
        size_t newsz = stack->cap * sizeof(Slice);
        Slice *new = realloc(stack->items, newsz);

        stack->items = new;
        stack->cap = newcap;
    }

    stack->items[stack->top] = item;
    stack->top += 1;
}

#endif // DRN_IMPL