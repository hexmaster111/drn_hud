
#include "drn.h"
#include "directive_list.h"

#ifndef DRN_RUNTIME_H
#define DRN_RUNTIME_H

typedef struct DrnRuntime
{
    DrnScript script;         // the script being ran, owns the arena and node memory
    DrnNode *now;             // ptr to current node being executed
    DirectiveList directives; // list of pointers to nodes in the script that should be rendered to drone
} DrnRuntime;

void DrnRuntime_TaskComplete(DrnRuntime *rt);
void DrnRuntime_True(DrnRuntime *rt);
void DrnRuntime_False(DrnRuntime *rt);
void DrnRuntime_Reset(DrnRuntime *rt);
void DrnRuntime_AcceptDirective(DrnRuntime *rt);

#endif //DRN_RUNTIME_H




#ifdef DRN_RUNTIME_IMPL
#undef DRN_RUNTIME_IMPL

// call before setting rt->now to next/inner
// check directives for ones that drop out of scope
void _DrnRuntime_CheckDirectiveScope(DrnRuntime *rt, DrnNode *next)
{
    DrnNode *directives = 0;
    DirectiveList_ItterState s = {0};
    while (DirectiveList_Itter(&rt->directives, &directives, &s))
    {
        if (next->token.indent < directives->token.indent)
        {
            DirectiveList_Remove(&rt->directives, directives);
        }
    }
}

// advanced from a task node
void DrnRuntime_TaskComplete(DrnRuntime *rt)
{
    if (!rt->now->next)
        return;

    _DrnRuntime_CheckDirectiveScope(rt, rt->now->next);
    rt->now = rt->now->next;
}

// enter the body of a condition node 
void DrnRuntime_True(DrnRuntime *rt)
{
    if (!rt->now->inner)
        return;

    _DrnRuntime_CheckDirectiveScope(rt, rt->now->inner);
    rt->now = rt->now->inner;
}

// enter the next node of a false condtion node
void DrnRuntime_False(DrnRuntime *rt) { DrnRuntime_TaskComplete(rt); }

// reset the runtime to the top of the starting file
void DrnRuntime_Reset(DrnRuntime *rt)
{
    rt->now = rt->script.start;
    DirectiveList_Clear(&rt->directives);
}

// accept directive into runtime, adding to directive list
void DrnRuntime_AcceptDirective(DrnRuntime *rt)
{
    if (!rt->now->next)
        return;

    DirectiveList_Add(&rt->directives, rt->now);
    _DrnRuntime_CheckDirectiveScope(rt, rt->now->next);
    rt->now = rt->now->next;
}

#endif //DRN_RUNTIME_H