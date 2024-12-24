#include "stage.h"
#include "common.h"
#include "draw.h"
#include "defs.h"

Entity * player;
SDL_Texture *ballTexture;
inline Stage stage;

void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.ballTail = &stage.ballHead;

    initPlayer();

    ballTexture = loadTexture(BALL_PATH);
}

static void initPlayer() {
    player = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;

    player->texture = loadTexture(MESSI_PATH);
    SDL_QueryTexture(player->texture, nullptr, nullptr, &player->w, &player->h);
}

static void logic() {
    doPlayer();
    doBullets();
}

static void doPlayer() {
    player->dx = player->dy = 0;

    if (player->reload > 0) {
        player->reload--;
    }

    if (app.keyboard[SDL_SCANCODE_UP]) {
        player->dy = -PLAYER_SPEED;
    }

    if (app.keyboard[SDL_SCANCODE_DOWN]) {
        player->dy = PLAYER_SPEED;
    }

    if (app.keyboard[SDL_SCANCODE_LEFT]) {
        player->dx = -PLAYER_SPEED;
    }

    if (app.keyboard[SDL_SCANCODE_RIGHT]) {
        player->dx = PLAYER_SPEED;
    }

    if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload == 0) {
        fireBullet();
    }

    player->x += player->dx;
    player->y += player->dy;
}

static void fireBullet() {
    auto *ball = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(ball, 0, sizeof(Entity));
    stage.ballTail->next = ball;
    stage.ballTail = ball;

    ball->x = player->x;
    ball->y = player->y;
    ball->dx = PLAYER_BULLET_SPEED;
    ball->health = 1;
    ball->texture = ballTexture;
    SDL_QueryTexture(ball->texture, NULL, NULL, &ball->w, &ball->h);

    ball->y += (player->h / 2) - (ball->h / 2);

    player->reload = 8;
}

static void doBullets() {
    Entity *prev = &stage.ballHead;

    for (Entity *b = stage.ballHead.next; b != NULL; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (b->x > SCREEN_WIDTH) {
            if (b == stage.ballTail) {
                stage.ballTail = prev;
            }

            prev->next = b->next;
            free(b);
            b = prev;
        }

        prev = b;
    }
}

static void draw() {
    drawPlayer();
    drawBullets();
}

static void drawPlayer() {
    blit(player->texture, player->x, player->y);
}

static void drawBullets() {
    for (const Entity *b = stage.ballHead.next; b != nullptr; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}
