
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

typedef struct DirectiveList_ItterState
{
    DirectiveListItem *itter;
    char ran_once;
} DirectiveList_ItterState;

void DirectiveList_DebugPrint(DirectiveList *this, FILE *fp);
void DirectiveList_Init(DirectiveList *this, Arena *arena);
void DirectiveList_Add(DirectiveList *this, DrnNode *node);
void DirectiveList_Remove(DirectiveList *this, DrnNode *node);

char DirectiveList_Itter(DirectiveList *this, DrnNode **out_item, DirectiveList_ItterState *state);

void DirectiveList_Clear(DirectiveList *this);

#endif // DIRECTIVELIST_H

#ifdef DIRECTIVELIST_IMPL
#undef DIRECTIVELIST_IMPL

// support removal of items while itterating forwards
char DirectiveList_Itter(DirectiveList *this, DrnNode **out_item, DirectiveList_ItterState *state)
{
    TODO("BUG-- this terminates itteration early on encountering the first null item, even if the next chain has an item");

    if (!state->ran_once)
    {
        state->itter = &this->start;
        state->ran_once = 1;
    }
    else if (state->itter->next == 0)
    {
        *out_item = 0;
        return 0;
    }
    else
    {
        state->itter = state->itter->next;
    }
    *out_item = state->itter->item;
    return *out_item != 0;
}

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

void DirectiveList_Test_Remove()
{
    Arena ar = ArenaNew(512);

    DrnNode *n0 = DrnNode_New((DrnToken){.indent = 1, .code = Slice_CStr("TASK: ")}, &ar);
    DrnNode *n1 = DrnNode_New((DrnToken){.indent = 2, .code = Slice_CStr("TASK: ")}, &ar);
    DrnNode *n2 = DrnNode_New((DrnToken){.indent = 3, .code = Slice_CStr("TASK: ")}, &ar);
    DrnNode *n3 = DrnNode_New((DrnToken){.indent = 4, .code = Slice_CStr("TASK: ")}, &ar);

    DirectiveList lst = {0};
    DirectiveList_Init(&lst, &ar);

    DirectiveList_Add(&lst, n0);
    DirectiveList_Add(&lst, n1);
    DirectiveList_Add(&lst, n2);
    DirectiveList_Add(&lst, n3);

    DirectiveList_DebugPrint(&lst, stdout);

    DirectiveList_ItterState is = {0};

    DrnNode *itter;
    while (DirectiveList_Itter(&lst, &itter, &is))
    {
        if (itter == n2)
        {
            DirectiveList_Remove(&lst, itter);
            printf("Removed n2\n");
        }
    }

    printf("-------------------\n");
    DirectiveList_DebugPrint(&lst, stdout);
    printf("-------------------\n");

    memset(&is, 0, sizeof(is));
    while (DirectiveList_Itter(&lst, &itter, &is))
    {
        printf("%d:%p\n", itter->token.indent, itter);
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