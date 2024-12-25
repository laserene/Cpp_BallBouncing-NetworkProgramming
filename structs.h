#ifndef STRUCTS_H
#define STRUCTS_H

#include "SDL2/SDL.h"
#include "defs.h"

struct Explosion {
    float x;
    float y;
    float dx;
    float dy;
    int r, g, b, a;
    Explosion *next;
};

struct Debris {
    float x;
    float y;
    float dx;
    float dy;
    SDL_Rect rect;
    SDL_Texture *texture;
    int life;
    Debris *next;
};

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

struct Entity {
    float x;
    float y;
    int w;
    int h;
    float dx;
    float dy;
    int health;
    int reload;
    int side;
    SDL_Texture *texture;
    Entity *next;
};

typedef struct {
    Entity fighterHead, *fighterTail;
    Entity ballHead, *ballTail;
    Explosion explosionHead, *explosionTail;
    Debris debrisHead, *debrisTail;
    Entity pointsHead, *pointsTail;
    int score;
} Stage;

typedef struct {
    int x;
    int y;
    int speed;
} Star;

#endif //STRUCTS_H
