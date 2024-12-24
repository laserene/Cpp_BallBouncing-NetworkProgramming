#include "SDL2/SDL.h"
#include "common.h"
#include "draw.h"

void prepareScene() {
    SDL_SetRenderDrawColor(app.renderer, 100, 200, 100, 255);
    SDL_RenderClear(app.renderer);
}

void presentScene() {
    SDL_RenderPresent(app.renderer);
}
