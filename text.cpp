#include "SDL2/SDL.h"
#include "text.h"

#include "defs.h"
#include "draw.h"

#define GLYPH_HEIGHT 28
#define GLYPH_WIDTH  18

SDL_Texture *fontTexture;
char drawTextBuffer[MAX_LINE_LENGTH];

void initFonts() {
    fontTexture = loadTexture("../font/font.png");
}

void drawText(int x, const int y, const int r, const int g, const int b, char *format, ...) {
    SDL_Rect rect;
    va_list args;

    memset(&drawTextBuffer, '\0', sizeof(drawTextBuffer));

    va_start(args, format);
    vsprintf(drawTextBuffer, format, args);
    va_end(args);

    const int len = strlen(drawTextBuffer);

    rect.w = GLYPH_WIDTH;
    rect.h = GLYPH_HEIGHT;
    rect.y = 0;

    SDL_SetTextureColorMod(fontTexture, r, g, b);

    for (int i = 0; i < len; i++) {
        if (const int c = drawTextBuffer[i]; c >= ' ' && c <= 'Z') {
            rect.x = (c - ' ') * GLYPH_WIDTH;

            blitRect(fontTexture, &rect, x, y);

            x += GLYPH_WIDTH;
        }
    }
}
