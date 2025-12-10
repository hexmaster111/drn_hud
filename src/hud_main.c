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
#define FONTID_HIGHREZ (1)
Font fonts[2] = {0};
int g_current_font = FONTID_HIGHREZ;
const int g_fontsize = 18;
const int g_fontspacing = 1;

#include "fonts.h"

void LoadFonts()
{

    Image f0 = (Image){.data = FONT_8_16_DATA, .width = FONT_8_16_WIDTH, .height = FONT_8_16_HEIGHT, .format = FONT_8_16_FORMAT, .mipmaps = 1};
    Image f1 = (Image){.data = FONT_16_32_DATA, .width = FONT_16_32_WIDTH, .height = FONT_16_32_HEIGHT, .format = FONT_16_32_FORMAT, .mipmaps = 1};

    fonts[FONTID_DEFAULT] = LoadFontFromImage(f0, MAGENTA, '!');
    fonts[FONTID_HIGHREZ] = LoadFontFromImage(f1, MAGENTA, '!');

    if (!IsFontValid(fonts[FONTID_DEFAULT]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        fonts[FONTID_DEFAULT] = GetFontDefault();
    }

    if (!IsFontValid(fonts[FONTID_HIGHREZ]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        fonts[FONTID_HIGHREZ] = GetFontDefault();
    }
}

void Text(const char *txt, Vector2 pos, Color c) { DrawTextEx(fonts[g_current_font], txt, pos, g_fontsize, g_fontspacing, c); }
Vector2 TextMeasure(const char *txt) { return MeasureTextEx(fonts[g_current_font], txt, g_fontsize, g_fontspacing); }

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
    DrnRuntime rt = {.script = Drn_LoadScriptFromFile("sample_drn/complex.drn")};

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

        int window_height = GetScreenHeight();
        int window_width = GetScreenWidth();
        Vector2 reset_size = TextMeasure("Reset");
        reset_size.x = window_width - reset_size.x;
        reset_size.y = window_height - reset_size.y;
        if (Button("Reset", reset_size, 0))
            DrnRuntime_Reset(&rt);

        Text(SliceToClitTmp(rt.now->token.code), (Vector2){20, 30}, BLACK);
        Text(TextFormat("%p", rt.now), (Vector2){20, 30 + g_fontsize + 5}, BLACK);
        EndDrawing();
    }
    CloseWindow();

    Drn_FreeScript(&rt.script);

    return 0;
}