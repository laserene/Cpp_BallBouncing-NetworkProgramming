#include "SDL2/SDL.h"
#include "common.h"
#include "draw.h"

void prepareScene() {
    SDL_SetRenderDrawColor(app.renderer, 96, 200, 255, 255);
    SDL_RenderClear(app.renderer);
}

void presentScene() {
    SDL_RenderPresent(app.renderer);
}
