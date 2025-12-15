
#include "arena.h"
#include "drn.h"

#ifndef DIRECTIVELIST_H
#define DIRECTIVELIST_H

typedef struct DirectiveListItem
{
    struct DirectiveListItem *next, *prev;
    DrnNode *item;
} DirectiveListItem;

typedef struct DirectiveList
{
    Arena *arena;
    DirectiveListItem start;
} DirectiveList;

void DirectiveList_DebugPrint(DirectiveList *this, FILE *fp);
void DirectiveList_Init(DirectiveList *this, Arena *arena);
void DirectiveList_Add(DirectiveList *this, DrnNode *node);
void DirectiveList_Remove(DirectiveList *this, DrnNode *node);

// TODO(perf, refactor to have a state object so we dont re-itter each time we call itter)
char DirectiveList_Itter(DirectiveList *this, DrnNode *out_item, int *state); // while(DirectiveList_Itter()) { ... }
void DirectiveList_Clear(DirectiveList *this);

#endif // DIRECTIVELIST_H

#ifdef DIRECTIVELIST_IMPL
#undef DIRECTIVELIST_IMPL

void DirectiveList_Clear(DirectiveList *this)
{
    DirectiveListItem *bottem = &this->start;

    for (;;)
    {
        bottem->item = 0;

        if (!bottem->next)
            break;

        bottem = bottem->next;
    }
}

char DirectiveList_Itter(DirectiveList *this, DrnNode *out_item, int *state)
{
    DirectiveListItem *bottem = &this->start;

    int i = 0;
    char hadone = 0;

    for (;;)
    {
        if (bottem->item)
        {
            if (*state == i)
            {
                hadone = 1;
                *out_item = *bottem->item;
                *state = *state + 1;
                break;
            }
            i++;
        }

        if (!bottem->next)
            break;

        bottem = bottem->next;
    }

    return hadone;
}

void DirectiveList_DebugPrint(DirectiveList *this, FILE *fp)
{
    DirectiveListItem *bottem = &this->start;

    int i = 0;
    for (;;)
    {
        if (bottem->item)
        {
            fprintf(fp, "%p %d: %d\n", bottem, i, bottem->item->token.indent);
            i++;
        }

        if (!bottem->next)
            break;

        bottem = bottem->next;
    }
}

void DirectiveList_Init(DirectiveList *this, Arena *arena)
{
    memset(this, 0, sizeof(DirectiveList));

    this->arena = arena;
}

// [Start] -> [Next] -> [next] -> null

void DirectiveList_Add(DirectiveList *this, DrnNode *node)
{
    DirectiveListItem *bottem = &this->start;

    while (bottem->next && bottem->item)
        bottem = bottem->next;

    if (!bottem->item)
    {
        // first / reuse this node alloc
        bottem->item = node;
        return;
    }

    DirectiveListItem *new_listitem = ArenaMalloc(this->arena, sizeof(DirectiveListItem));
    new_listitem->item = node;
    new_listitem->prev = bottem;
    bottem->next = new_listitem;
}

void DirectiveList_Remove(DirectiveList *this, DrnNode *node)
{
    // we dont get rid of contanors (cope for arena being a bump allocator), we just null out item, for re-use
    DirectiveListItem *bottem = &this->start;

    for (;;)
    {
        if (bottem->item == node)
        {
            bottem->item = 0;
            return;
        }

        if (!bottem->next)
        {
            printf("[WARN] : Unable to find node %p to remove!\n", node);
            return;
        }

        bottem = bottem->next;
    }
}

void DirectiveList_Test()
{
    Arena arena = ArenaNew(1024);
    DirectiveList dl;
    DirectiveList_Init(&dl, &arena);

    DrnNode *test1 = ArenaMalloc(&arena, sizeof(DrnNode));
    test1->token.indent = 1;
    DrnNode *test2 = ArenaMalloc(&arena, sizeof(DrnNode));
    test2->token.indent = 2;
    DrnNode *test3 = ArenaMalloc(&arena, sizeof(DrnNode));
    test3->token.indent = 3;
    DirectiveList_Add(&dl, test1);
    DirectiveList_Add(&dl, test2);
    DirectiveList_Add(&dl, test3);
    DirectiveList_DebugPrint(&dl, stdout);
    printf("----------------\n");
    DirectiveList_Remove(&dl, test2);
    DirectiveList_DebugPrint(&dl, stdout);
    printf("----------------\n");
    DirectiveList_Remove(&dl, test1);
    DirectiveList_DebugPrint(&dl, stdout);
    printf("----------------\n");
    DirectiveList_Remove(&dl, test2);
    DirectiveList_DebugPrint(&dl, stdout);

    ArenaFree(&arena);
}

#endif // DIRECTIVELIST_IMPL