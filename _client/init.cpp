#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "common.h"
#include "defs.h"
#include "init.h"

void initSDL() {
    constexpr int rendererFlags = SDL_RENDERER_ACCELERATED;
    constexpr int windowFlags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // Window
    app.window = SDL_CreateWindow("Arkanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                  SCREEN_HEIGHT, windowFlags);
    if (!app.window) {
        printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Renderer
    app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);
    if (!app.renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    // Font
    if (TTF_Init() < 0) {
        printf("Couldn't initialize TTF: %s\n", TTF_GetError());
    }

    // Img
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) < 0) {
        printf("Couldn't initialize SDL_image: %s\n", IMG_GetError());
    }

    SDL_ShowCursor(0);
}
