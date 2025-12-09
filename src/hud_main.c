#include <raylib.h>
#include <raymath.h>

#define ARENA_IMPL
#include "arena.h"

#define DRN_IMPL
#include "drn.h"

#define SLICE_STACK_IMPL
#include "slice_stack.h"

// ---------------------------- Runtime
typedef struct DrnRuntime
{
    DrnScript script;
    DrnNode *now;
} DrnRuntime;

// Task Node
void DrnRuntime_TaskComplete(DrnRuntime *rt)
{
    if (rt->now->next)
        rt->now = rt->now->next;
}

// Condition Node
void DrnRuntime_True(DrnRuntime *rt)
{
    if (rt->now->inner)
        rt->now = rt->now->inner;
}

void DrnRuntime_False(DrnRuntime *rt) { DrnRuntime_TaskComplete(rt); }

void DrnRuntime_Reset(DrnRuntime *rt) { rt->now = rt->script.start; }

// --------------------------- font stuff
#define FONTID_DEFAULT (0)
Font fonts[1] = {0};

#include "fonts.h"

void LoadFonts()
{

    Image img = (Image){
        .data = FONT_8_16_DATA,
        .width = FONT_8_16_WIDTH,
        .height = FONT_8_16_HEIGHT,
        .format = FONT_8_16_FORMAT,
        .mipmaps = 1};

    fonts[FONTID_DEFAULT] = LoadFontFromImage(img, MAGENTA, '!');

    if (!IsFontValid(fonts[FONTID_DEFAULT]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        fonts[FONTID_DEFAULT] = GetFontDefault();
    }
}

const int g_fontsize = 18;
const int g_fontspacing = 1;

void Text(const char *txt, Vector2 pos, Color c)
{
    DrawTextEx(fonts[FONTID_DEFAULT],
               txt,
               pos,
               g_fontsize, g_fontspacing, c);
}

Vector2 TextMeasure(const char *txt)
{
    return MeasureTextEx(fonts[FONTID_DEFAULT], txt, g_fontsize, g_fontspacing);
}

bool Button(const char *label, Vector2 pos, Vector2 *opt_size)
{
    Vector2 sz = TextMeasure(label);
    if (opt_size)
        *opt_size = sz;

    DrawRectangleV(pos, sz, GRAY);
    Text(label, pos, ORANGE);

    bool over = CheckCollisionPointRec(GetMousePosition(), (Rectangle){
                                                               .x = pos.x,
                                                               .y = pos.y,
                                                               .width = sz.x,
                                                               .height = sz.y,
                                                           });

    return over && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

// returns staticly allocated string
char *SliceToClit(Slice s)
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
    DrnRuntime rt = {.script = LoadDrnScriptFromFile("sample_drn/simple.txt")};

    rt.now = rt.script.start;

    if (!rt.script.start)
    {
        return 1;
    }

    InitWindow(800, 600, "DRN");
    SetTargetFPS(60);
    LoadFonts();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);

        Vector2 sz, pos = {10, 10};
        const float pad = 5;

        if (Button("Task Complete", pos, &sz))
            DrnRuntime_TaskComplete(&rt);

        pos.x += sz.x + pad;

        if (Button("True", pos, &sz))
            DrnRuntime_True(&rt);

        pos.x += sz.x + pad;

        if (Button("False", pos, &sz))
            DrnRuntime_False(&rt);

        pos.x += sz.x + pad;

        if (Button("Reset", pos, &sz))
            DrnRuntime_Reset(&rt);

        Text(SliceToClit(rt.now->token.code), (Vector2){20, 30}, BLACK);

        EndDrawing();
    }

    CloseWindow();

    ArenaFree(&rt.script.arena);

    return 0;
}