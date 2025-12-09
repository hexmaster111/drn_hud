#ifndef SLICE_STACK_H
#define SLICE_STACK_H

#include "slice.h"

typedef struct
{
    Slice *items;
    size_t cap, top;
} SliceStack, SliceList;

void SliceStack_Push(SliceStack *, Slice);
void SliceStack_Clear(SliceStack *);
Slice SliceStack_Pop(SliceStack *);

#endif //SLICE_STACK_H

#ifdef SLICE_STACK_IMPL
#undef SLICE_STACK_IMPL

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


#endif //SLICE_STACK_IMPL