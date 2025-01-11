#ifndef STRUCTS_H
#define STRUCTS_H

#include "SDL2/SDL.h"

#include "defs.h"

typedef struct {
    void (*logic)();

    void (*draw)();
} Delegate;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    Delegate delegate;
    int keyboard[MAX_KEYBOARD_KEYS];
} App;

#endif //STRUCTS_H
