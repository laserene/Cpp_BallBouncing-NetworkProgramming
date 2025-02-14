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
    void (*logic)(int sock, const char *username);

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
    Pod buffList[NUM_BUFF - 3];
    Pod debuffList[NUM_DEBUFF];
    int score;
} Stage;

typedef struct {
    int x;
    int y;
    int speed;
} Star;

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

enum Biome { HALLOW, SNOW, SPACE, RUSTY};
typedef struct{
    char background[1024];
    char enemy[1024];
    char enemy_bullet[1024];
    int enemy_health;
    int enemy_bullet_delta;
} BiomeStat;

enum Screen { WELCOME, LOGIN, SIGNUP, MENU, CHARACTER, BIOME, ROOM, PLAY, LEADERBOARD, EXIT, POSTGAME };

#endif //STRUCTS_H
