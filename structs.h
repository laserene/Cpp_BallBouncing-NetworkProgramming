#ifndef STRUCTS_H
#define STRUCTS_H

#include "SDL2/SDL.h"

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    int up;
    int down;
    int left;
    int right;
    int fire;
} App;

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    int health;
    SDL_Texture *texture;
} Entity;

#endif //STRUCTS_H
