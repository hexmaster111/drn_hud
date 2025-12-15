#ifndef FONTS_H
#define FONTS_H

#include <raylib.h>

#define FONTID_DEFAULT (0)
#define FONTID_HIGHREZ (1)

Font g_fonts[2] = {0};
int g_current_font = FONTID_HIGHREZ;
const int g_fontsize = 18;
const int g_fontspacing = 1;


#define FONT_16_32_WIDTH 545
#define FONT_16_32_HEIGHT 991
#define FONT_16_32_FORMAT 7 // raylib internal pixel format

extern unsigned char FONT_16_32_DATA[2160380];

#define FONT_8_16_WIDTH 289
#define FONT_8_16_HEIGHT 511
#define FONT_8_16_FORMAT 7 // raylib internal pixel format

extern unsigned char FONT_8_16_DATA[590716];

// loads and unloads fonts into global fonts var
void UnloadFonts();
void LoadFonts();
#endif // FONTS_H


#ifdef FONTS_IMPL
#undef FONTS_IMPL

void UnloadFonts()
{
    for (size_t i = 0; i < sizeof(g_fonts) / sizeof(g_fonts[0]); i++)
    {
        UnloadFont(g_fonts[i]);
    }
}

void LoadFonts()
{

    Image f0 = (Image){.data = FONT_8_16_DATA, .width = FONT_8_16_WIDTH, .height = FONT_8_16_HEIGHT, .format = FONT_8_16_FORMAT, .mipmaps = 1};
    Image f1 = (Image){.data = FONT_16_32_DATA, .width = FONT_16_32_WIDTH, .height = FONT_16_32_HEIGHT, .format = FONT_16_32_FORMAT, .mipmaps = 1};

    g_fonts[FONTID_DEFAULT] = LoadFontFromImage(f0, MAGENTA, '!');
    g_fonts[FONTID_HIGHREZ] = LoadFontFromImage(f1, MAGENTA, '!');

    if (!IsFontValid(g_fonts[FONTID_DEFAULT]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        g_fonts[FONTID_DEFAULT] = GetFontDefault();
    }

    if (!IsFontValid(g_fonts[FONTID_HIGHREZ]))
    {
        TraceLog(LOG_WARNING, "Failed to load font!");
        g_fonts[FONTID_HIGHREZ] = GetFontDefault();
    }
}

#endif //FONTS_IMPL