
// drn.h
#ifndef DRN_H
#define DRN_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "arena.h"
#include "slice.h"
#include "slice_stack.h"

typedef struct DrnToken
{
    Slice code;
    int indent;
} DrnToken;

struct DrnNode
{
    struct DrnToken token;
    struct DrnNode *next, *inner, *parent;

    enum DrnNodeKind
    {
        DNK_UNKNOWN = 0,
        DNK_TASK,
        DNK_CONDITION,
        DNK_END_OF_FILE
    } type;
};
typedef struct DrnNode DrnNode;
DrnNode *DrnNode_New(DrnToken tok, Arena *arena);

typedef struct
{
    Arena arena;
    DrnNode *start, *eof;
} DrnScript;

DrnScript Drn_LoadScriptFromFile(const char *path);
void Drn_FreeScript(DrnScript *scr);

typedef struct DrnLexer
{
    char *end;
    char *now;

    char *filename;
    size_t line;
} DrnLexer;

Slice DrnLex_ReadToNewline(DrnLexer *s);
void DrnLex_GetAndStripIdent(Slice *s, int *out_indent);
int DrnLex_HasMore(DrnLexer *lxr);
DrnToken DrnLex_Next(DrnLexer *s);
DrnToken DrnLex_Peek(struct DrnLexer *s);

typedef struct DrnInserter
{
    DrnNode *node;
    DrnNode *eof_node;
    int indent;
} DrnInserter;

void DrnInserter_Insert(DrnInserter *this, DrnNode *new_node);

#endif // DRN_H

// drn.c
#ifdef DRN_IMPL
#undef DRN_IMPL

#define TODO(MSG)                                                   \
    do                                                              \
    {                                                               \
        printf("%s:%d TODO: %s\n", __FILE_NAME__, __LINE__, (MSG)); \
        abort();                                                    \
    } while (0);

void Drn_FreeScript(DrnScript *scr)
{
    ArenaFree(&scr->arena);
    scr->eof = 0;
    scr->start = 0;
}

Slice DrnLex_ReadToNewline(DrnLexer *s)
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
    while (memcmp(s->base, "    ", 4) == 0)
    {
        s->base += 4;
        s->len -= 4;

        *out_indent += 1;
    }
}

int DrnLex_HasMore(DrnLexer *lxr)
{
    return lxr->now != lxr->end;
}

DrnToken DrnLex_Next(DrnLexer *s)
{
SKIP:
    int indent;

    Slice line = DrnLex_ReadToNewline(s);
    DrnLex_GetAndStripIdent(&line, &indent);

    if (line.base[0] == '#' || line.base[0] == '\n' || line.len == 0)
    {

        if (!DrnLex_HasMore(s))
            return (DrnToken){0};

        // empty line, comment, newline
        // comment line, skip DRONES NEVER PARSE THE COMMENTS!
        goto SKIP;
    }

    return (DrnToken){.code = line, .indent = indent};
}

DrnToken DrnLex_Peek(struct DrnLexer *s)
{
    // ineficnet af, we re-do our work ever peek, but, this is fast to impl
    DrnLexer bck = *s;
    DrnToken peek = DrnLex_Next(s);
    *s = bck;
    return peek;
}

DrnNode *DrnNode_New(DrnToken tok, Arena *arena)
{
    DrnNode *n = ArenaMalloc(arena, sizeof(DrnNode));

    n->token = tok;
    n->inner = 0;
    n->next = 0;

    if (strncmp("TASK", tok.code.base, 4) == 0)
    {
        n->type = DNK_TASK;
    }
    else if (strncmp("CONDITION", tok.code.base, 9) == 0)
    {
        n->type = DNK_CONDITION;
    }

    return n;
}

void DrnInserter_Insert(DrnInserter *this, DrnNode *new_node)
{
    if (new_node->token.indent == this->indent)
    {
        this->node->next = new_node;
        new_node->parent = this->node;
    }
    else if (new_node->token.indent > this->indent)
    {
        this->node->inner = new_node;
        new_node->parent = this->node;
        this->indent = new_node->token.indent;
    }
    else if (new_node->token.indent < this->indent)
    {
        DrnNode *parent = this->node->parent;
        while (parent->token.indent != new_node->token.indent)
        {
            parent = parent->parent;
        }

        parent->next = new_node;
        new_node->parent = parent;
        this->node->next = new_node;
        this->indent = new_node->token.indent;
    }

    this->node = new_node;
}

DrnScript LoadDrnScriptFromFile(const char *path)
{
    if (!FileExists(path))
    {
        printf("File not found! %s\n", path);
        return (DrnScript){};
    }

    DrnScript scr = {0};
    scr.arena = ArenaNew(1 << 16);
    size_t filetextlen = 0;
    char *filetext = LoadFileTextArena(&scr.arena, path, &filetextlen);

    DrnLexer lxr = {
        .filename = StringCopyArena(&scr.arena, path),
        .now = filetext,
        .end = filetext + filetextlen,
        .line = 0,
    };

    DrnToken tok = DrnLex_Next(&lxr);
    scr.start = DrnNode_New(tok, &scr.arena);
    DrnInserter ins = {
        .indent = 0,
        .node = scr.start,
    };

    const DrnToken eof_token = {.code = Slice_CStr(StringCopyArena(&scr.arena, "[END OF FILE]"))};
    ins.eof_node = DrnNode_New(eof_token, &scr.arena);
    ins.eof_node->type = DNK_END_OF_FILE;

    while (DrnLex_HasMore(&lxr))
    {
        tok = DrnLex_Next(&lxr);

        if (tok.code.len == 0)
            continue;

        DrnNode *new_node = DrnNode_New(tok, &scr.arena);
        DrnInserter_Insert(&ins, new_node);
    }

    ins.node->next = ins.eof_node;

    return scr;
}

#endif // DRN_IMPL