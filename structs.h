#ifndef STRUCTS_H
#define STRUCTS_H

#include "SDL2/SDL.h"
#include "defs.h"
#include "buff.h"
#include "debuff.h"

struct Buff {
    int id = -1;
    int time_to_live;
    SDL_Texture *texture;
};

struct Debuff {
    int id = -1;
    int time_to_live;
    SDL_Texture *texture;
};

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
    float health;
    int reload;
    int side;
    int buff_type = 0;
    SDL_Texture *texture;
    Entity *next;
};

typedef struct {
    Entity fighterHead, *fighterTail;
    Entity ballHead, *ballTail;
    Explosion explosionHead, *explosionTail;
    Debris debrisHead, *debrisTail;
    Entity pointsHead, *pointsTail;
    Buff buffList[NUM_BUFF];
    Debuff debuffList[NUM_DEBUFF];
    int score;
} Stage;

typedef struct {
    int x;
    int y;
    int speed;
} Star;

typedef struct {
    int recent;
    int score;
} Highscore;

typedef struct {
    Highscore highscore[NUM_HIGHSCORES];
} Highscores;

typedef struct {
    int enemy_delta_bullet = 0;
    float player_delta_bullet = 0;
    int player_delta = 0;
    int player_delta_dx = 0;
    int player_delta_dy = 0;
    int player_delta_luck = 0;
    int alpha = 255;
    int enforced_bullet = 0;
} Stat;

#endif //STRUCTS_H
