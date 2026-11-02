#ifndef SLICE_STACK_H
#define SLICE_STACK_H

#include "slice.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    Slice *items;
    size_t cap, top;
} SliceStack, SliceList;

void SliceStack_Push(SliceStack *, Slice);
void SliceStack_PushBack(SliceStack *, Slice); // Inserts at the front (index 0)
void SliceStack_Clear(SliceStack *);
Slice SliceStack_Pop(SliceStack *);

#endif //SLICE_STACK_H

#ifdef SLICE_STACK_IMPL
#undef SLICE_STACK_IMPL

Slice SliceStack_Pop(SliceStack *stack) {
    if (!stack->items) return (Slice){0};
    if (stack->top == 0) return (Slice){0};
    stack->top -= 1;
    Slice s = stack->items[stack->top];
    return s;
}

void SliceStack_Push(SliceStack *stack, Slice item) {
    if (!stack->items) {
        // new stack, allocate array
        const int inital_cap = 5;
        const size_t sz = sizeof(Slice) * inital_cap;
        stack->items = malloc(sz);
        memset(stack->items, 0, sz);
        stack->cap = inital_cap;
        stack->top = 0;
    }
    if (stack->top + 1 > stack->cap) {
        // grow array (Fixed bug: multiplied newcap correctly for newsz)
        size_t newcap = stack->cap * 2;
        size_t newsz = newcap * sizeof(Slice);
        Slice *new = realloc(stack->items, newsz);
        stack->items = new;
        stack->cap = newcap;
    }
    stack->items[stack->top] = item;
    stack->top += 1;
}

void SliceStack_PushBack(SliceStack *stack, Slice item) {
    if (!stack->items) {
        // new stack, initialize and push normally
        SliceStack_Push(stack, item);
        return;
    }
    
    if (stack->top + 1 > stack->cap) {
        // grow array
        size_t newcap = stack->cap * 2;
        size_t newsz = newcap * sizeof(Slice);
        Slice *new = realloc(stack->items, newsz);
        stack->items = new;
        stack->cap = newcap;
    }

    // Shift all existing elements to the right by 1 position
    memmove(&stack->items[1], &stack->items[0], stack->top * sizeof(Slice));
    
    // Insert the new item at the front
    stack->items[0] = item;
    stack->top += 1;
}

#endif //SLICE_STACK_IMPL
