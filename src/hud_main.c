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

const int textLineSpacing = 1;

// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
// lifed from raylib, modfied to work with slices
void DrawTextExHG(Font font, Slice text, Vector2 position, float fontSize, float spacing, Color tint)
{
    if (font.texture.id == 0)
        font = GetFontDefault(); // Security check in case of not valid font

    int size = text.len; // Total size in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;    // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f; // Offset X to next character to draw

    float scaleFactor = fontSize / font.baseSize; // Character quad scaling factor

    for (int i = 0; i < size;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text.base[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
        {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint, (Vector2){position.x + textOffsetX, position.y + textOffsetY}, fontSize, tint);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
            else
                textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
        }

        i += codepointByteCount; // Move text bytes counter to next codepoint
    }
}

// lifed from raylib, modfied to work with slices
Vector2 MeasureTextExHG(Font font, Slice text, float fontSize, float spacing)
{
    Vector2 textSize = {0};

    if ((font.texture.id == 0) || (text.base == NULL) || (text.base[0] == '\0'))
        return textSize; // Security check

    int size = text.len;     // Get size in bytes of text
    int tempByteCounter = 0; // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f; // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize / (float)font.baseSize;

    int letter = 0; // Current character
    int index = 0;  // Index position in sprite font

    for (int i = 0; i < size;)
    {
        byteCounter++;

        int codepointByteCount = 0;
        letter = GetCodepointNext(&text.base[i], &codepointByteCount);
        index = GetGlyphIndex(font, letter);

        i += codepointByteCount;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX > 0)
                textWidth += font.glyphs[index].advanceX;
            else
                textWidth += (font.recs[index].width + font.glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth)
                tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textHeight += (fontSize + textLineSpacing);
        }

        if (tempByteCounter < byteCounter)
            tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth)
        tempTextWidth = textWidth;

    textSize.x = tempTextWidth * scaleFactor + (float)((tempByteCounter - 1) * spacing);
    textSize.y = textHeight;

    return textSize;
}

void Text(Slice txt, Vector2 pos, Color c) { DrawTextExHG(g_fonts[g_current_font], txt, pos, g_fontsize, g_fontspacing, c); }
Vector2 TextMeasure(Slice txt) { return MeasureTextExHG(g_fonts[g_current_font], txt, g_fontsize, g_fontspacing); }

bool Button(Slice label, Vector2 pos, Vector2 *opt_size)
{
    Vector2 sz = TextMeasure(label);
    if (opt_size)
        *opt_size = sz;

    DrawRectangleV(pos, sz, GRAY);
    Text(label, pos, ORANGE);
    bool over = CheckCollisionPointRec(GetMousePosition(), (Rectangle){.x = pos.x, .y = pos.y, .width = sz.x, .height = sz.y});
    return over && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
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
            if (Button(SLICE_CLIT("Task Complete"), pos, &sz))
                DrnRuntime_TaskComplete(&rt);
            pos.x += sz.x + pad;
        }
        else if (rt.now->type == DNK_CONDITION)
        {
            if (Button(SLICE_CLIT("True"), pos, &sz))
                DrnRuntime_True(&rt);
            pos.x += sz.x + pad;

            if (Button(SLICE_CLIT("False"), pos, &sz))
                DrnRuntime_False(&rt);
            pos.x += sz.x + pad;
        }
        else if (rt.now->type == DNK_DIRECTIVE)
        {
            if (Button(SLICE_CLIT("Acknowledge"), pos, &sz))
                DrnRuntime_AcceptDirective(&rt);
            pos.x += sz.x + pad;
        }

        int window_height = GetScreenHeight();
        int window_width = GetScreenWidth();
        Vector2 reset_size = TextMeasure(SLICE_CLIT("Reset"));
        reset_size.x = window_width - reset_size.x;
        reset_size.y = window_height - reset_size.y;
        if (Button(SLICE_CLIT("Reset"), reset_size, 0))
            DrnRuntime_Reset(&rt);

        Text(rt.now->token.code, (Vector2){20, 30}, BLACK);

        DrnNode *directive = 0;
        int i = 0;
        DirectiveList_ItterState itterstate = {0};
        while (DirectiveList_Itter(&rt.directives, &directive, &itterstate))
        {
            i += 1;
            Text(directive->token.code, (Vector2){20, 30 + g_fontsize + (i * g_fontsize) + pad}, BLACK);
        }

        EndDrawing();
    }
    UnloadFonts();

    CloseWindow();

    Drn_FreeScript(&rt.script);

    return 0;
}