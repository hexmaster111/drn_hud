#include <raylib.h>
#include <raymath.h>

#define ARENA_IMPL
#include "arena.h"

#define DRN_IMPL
#include "drn.h"

#define SLICE_STACK_IMPL
#include "slice_stack.h"

#define SLICE_IMPL
#include "slice.h"

#define DIRECTIVELIST_IMPL
#include "directive_list.h"

#define FONTS_IMPL
#include "fonts.h"

// ---------------------------- Runtime

typedef struct DrnRuntime
{
    DrnScript script;
    DrnNode *now;
    DirectiveList directives;
} DrnRuntime;

// call before setting rt->now to next/inner
// check directives for ones that drop out of scope
void _DrnRuntime_Step(DrnRuntime *rt, DrnNode *next)
{
    printf("%d->%d\n", rt->now->token.indent, next->token.indent);

    // if we are dropping down lower then any directives, we can cancel them
}

// Task Node
void DrnRuntime_TaskComplete(DrnRuntime *rt)
{
    if (!rt->now->next)
        return;

    _DrnRuntime_Step(rt, rt->now->next);
    rt->now = rt->now->next;
}

// Condition Node
void DrnRuntime_True(DrnRuntime *rt)
{
    if (!rt->now->inner)
        return;

    _DrnRuntime_Step(rt, rt->now->inner);
    rt->now = rt->now->inner;
}

void DrnRuntime_False(DrnRuntime *rt) { DrnRuntime_TaskComplete(rt); }

void DrnRuntime_Reset(DrnRuntime *rt)
{
    rt->now = rt->script.start;
    DirectiveList_Clear(&rt->directives);
}

void DrnRuntime_AcceptDirective(DrnRuntime *rt)
{
    if (!rt->now->next)
        return;

    DirectiveList_Add(&rt->directives, rt->now);
    _DrnRuntime_Step(rt, rt->now->next);
    rt->now = rt->now->next;
}

// --------------------------- UI stuff

void Text(const char *txt, Vector2 pos, Color c) { DrawTextEx(g_fonts[g_current_font], txt, pos, g_fontsize, g_fontspacing, c); }
Vector2 TextMeasure(const char *txt) { return MeasureTextEx(g_fonts[g_current_font], txt, g_fontsize, g_fontspacing); }

bool Button(const char *label, Vector2 pos, Vector2 *opt_size)
{
    Vector2 sz = TextMeasure(label);
    if (opt_size)
        *opt_size = sz;

    DrawRectangleV(pos, sz, GRAY);
    Text(label, pos, ORANGE);
    bool over = CheckCollisionPointRec(GetMousePosition(), (Rectangle){.x = pos.x, .y = pos.y, .width = sz.x, .height = sz.y});
    return over && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

// returns staticly allocated string
char *SliceToClitTmp(Slice s)
{
    static char *memory = 0;
    static int memory_len = 0;
    if (s.len + 1 > memory_len)
    {
        memory = realloc(memory, s.len + 1);
        memory_len = s.len + 1;
    }
    memset(memory, 0, memory_len);
    strncpy(memory, s.base, s.len);
    return memory;
}

int main(int argc, char *argv[])
{
    DrnRuntime rt = {.script = Drn_LoadScriptFromFile("sample_drn/directive.drn")};
    DirectiveList_Init(&rt.directives, &rt.script.arena);

    rt.now = rt.script.start;
    if (!rt.script.start)
    {
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetTargetFPS(60);
    InitWindow(512, 160, "DRN");

    LoadFonts();
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        Vector2 sz, pos = {10, 10};
        const float pad = 5;

        if (rt.now->type == DNK_TASK)
        {
            if (Button("Task Complete", pos, &sz))
                DrnRuntime_TaskComplete(&rt);
            pos.x += sz.x + pad;
        }
        else if (rt.now->type == DNK_CONDITION)
        {
            if (Button("True", pos, &sz))
                DrnRuntime_True(&rt);
            pos.x += sz.x + pad;

            if (Button("False", pos, &sz))
                DrnRuntime_False(&rt);
            pos.x += sz.x + pad;
        }
        else if (rt.now->type == DNK_DIRECTIVE)
        {
            if (Button("Acknowledge", pos, &sz))
                DrnRuntime_AcceptDirective(&rt);
            pos.x += sz.x + pad;
        }

        int window_height = GetScreenHeight();
        int window_width = GetScreenWidth();
        Vector2 reset_size = TextMeasure("Reset");
        reset_size.x = window_width - reset_size.x;
        reset_size.y = window_height - reset_size.y;
        if (Button("Reset", reset_size, 0))
            DrnRuntime_Reset(&rt);

        Text(SliceToClitTmp(rt.now->token.code), (Vector2){20, 30}, BLACK);
        Text(TextFormat("%d  %p", rt.now->token.indent, rt.now), (Vector2){20, 30 + g_fontsize + pad}, BLACK);

        DrnNode item = {0};
        int i = 0;
        while (DirectiveList_Itter(&rt.directives, &item, &i))
        {
            Text(SliceToClitTmp(item.token.code), (Vector2){20, 30 + g_fontsize + (i * g_fontsize) + pad}, BLACK);
        }

        EndDrawing();
    }
    CloseWindow();

    UnloadFonts();

    Drn_FreeScript(&rt.script);

    return 0;
}