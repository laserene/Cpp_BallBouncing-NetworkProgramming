#include "text.h"
#include "common.h"
#include "draw.h"

#define GLYPH_HEIGHT 28
#define GLYPH_WIDTH  18

static SDL_Texture *fontTexture;
static char drawTextBuffer[MAX_LINE_LENGTH];

void initFonts() {
    fontTexture = loadTexture(FONT_TEXTURE);
}

void drawText(int x, const int y, const int r, const int g, const int b, const char *format, ...) {
    SDL_Rect rect;
    va_list args;

    memset(&drawTextBuffer, '\0', sizeof(drawTextBuffer));

    va_start(args, format);
    vsprintf(drawTextBuffer, format, args);
    va_end(args);

    const int len = strlen(drawTextBuffer);

    for (int i = 0; drawTextBuffer[i] != '\0'; ++i) {
        drawTextBuffer[i] = toupper(drawTextBuffer[i]);
    }

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
