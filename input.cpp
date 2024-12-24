#include "SDL2/SDL.h"
#include "common.h"
#include "input.h"

void doInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                doKeyDown();
                break;

            case SDL_KEYUP:
                doKeyUp();
                break;

            case SDL_QUIT:
                exit(0);
                break;

            default:
                break;
        }
    }
}

static void doKeyDown() {

}

static void doKeyUp() {
}
