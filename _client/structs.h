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
    // Passsing sock and read_fds to handle server messages
    void (*logic)(int sock);

    void (*draw)();
} Delegate;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    Delegate delegate;
    int keyboard[MAX_KEYBOARD_KEYS];
} App;

typedef struct {
    int id;
    int health;
    SDL_Texture *texture;
} Pod;

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
    int pod_id;
    SDL_Texture *texture;
    Entity *next;
};

typedef struct {
    Entity fighterHead, *fighterTail;
    Entity bulletHead, *bulletTail;
    Explosion explosionHead, *explosionTail;
    Debris debrisHead, *debrisTail;
    Entity pointsHead, *pointsTail;
    // there are 3 instant-consuming buffs: FROZEN, HEART & REFRESH
    Pod buffList[NUM_BUFF - 3];
    Pod debuffList[NUM_DEBUFF];
    int score;
} Stage;

typedef struct {
    int x;
    int y;
    int speed;
} Star;

enum Screen { WELCOME, LOGIN, SIGNUP, MENU, CHARACTER, BIOME, ROOM, PLAY, LEADERBOARD, EXIT };

#endif //STRUCTS_H
