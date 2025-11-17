
// drn.h
#ifndef DRN_H
#define DRN_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char *base;
    size_t len;
} Slice;

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
} DrnNodeKind;

struct DrnNode
{
    DrnNodeKind kind;
    Slice code; // without TASK: CONDITION:

    struct DrnNode *next;
    struct DrnNode *inner;
};
typedef struct DrnNode DrnNode;

typedef struct
{
    DrnNode start;
} DrnScript;

typedef struct
{
    SliceStack task_stack;
} DrnExec;

#endif // DRN_H

// drn.c
#ifdef DRN_IMPL
#undef DRN_IMPL

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

#ifdef DRN_IMPL_TEST
#undef DRN_IMPL_TEST

int main()
{

    return 0;
}

#endif // DRN_IMPL_TEST