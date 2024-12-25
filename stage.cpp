#include "stage.h"
#include "common.h"
#include "draw.h"
#include "util.h"
#include "defs.h"

Entity *player;
SDL_Texture *playerTexture;
SDL_Texture *alienBulletTexture;
SDL_Texture *ballTexture;
SDL_Texture *enemyTexture;
inline Stage stage;
int enemySpawnTimer;
int stageResetTimer;

void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.ballTail = &stage.ballHead;

    initPlayer();

    ballTexture = loadTexture(BALL_PATH);
    enemyTexture = loadTexture(ENEMY_PATH);
    alienBulletTexture = loadTexture("../gfx/enemy_bomb.png");
    playerTexture = loadTexture("../gfx/clothier.png");

    resetStage();
}

static void initPlayer() {
    player = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;
    player->side = SIDE_PLAYER;
    player->health = 3;

    player->x = 100;
    player->y = 100;

    player->texture = playerTexture;
    SDL_QueryTexture(player->texture, nullptr, nullptr, &player->w, &player->h);
}

static void resetStage() {
    Entity *e;

    while (stage.fighterHead.next) {
        e = stage.fighterHead.next;
        stage.fighterHead.next = e->next;
        free(e);
    }

    while (stage.ballHead.next) {
        e = stage.ballHead.next;
        stage.ballHead.next = e->next;
        free(e);
    }

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.ballTail = &stage.ballHead;

    initPlayer();

    enemySpawnTimer = 0;

    stageResetTimer = FPS * 2;
}

static void logic() {
    doPlayer();
    doFighters();
    doBullets();
    spawnEnemies();
    doEnemies();
    clipPlayer();

    if (player == nullptr && --stageResetTimer <= 0) {
        resetStage();
    }
}

static void doFighters() {
    Entity *prev = &stage.fighterHead;

    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (e != player && e->x < -e->w) {
            e->health = 0;
        }

        if (e->health == 0) {
            if (e == player) {
                player = nullptr;
            }

            if (e == stage.fighterTail) {
                stage.fighterTail = prev;
            }

            prev->next = e->next;
            free(e);
            e = prev;
        }

        prev = e;
    }
}

static void spawnEnemies() {
    if (--enemySpawnTimer <= 0) {
        auto *enemy = static_cast<Entity *>(malloc(sizeof(Entity)));
        memset(enemy, 0, sizeof(Entity));
        stage.fighterTail->next = enemy;
        stage.fighterTail = enemy;
        enemy->side = SIDE_ALIEN;

        enemy->health = 1;
        enemy->x = SCREEN_WIDTH;
        enemy->y = rand() % SCREEN_HEIGHT;
        enemy->texture = enemyTexture;
        SDL_QueryTexture(enemy->texture, nullptr, nullptr, &enemy->w, &enemy->h);

        enemy->dx = -(2 + (rand() % 4));

        enemySpawnTimer = 30 + (rand() % 60);
        enemy->reload = FPS * (1 + (rand() % 3));
    }
}

static void clipPlayer() {
    if (player != nullptr) {
        if (player->x < 0) {
            player->x = 0;
        }

        if (player->y < 0) {
            player->y = 0;
        }

        if (player->x > SCREEN_WIDTH / 3) {
            player->x = SCREEN_WIDTH / 3;
        }

        if (player->y > SCREEN_HEIGHT - player->h) {
            player->y = SCREEN_HEIGHT - player->h;
        }
    }
}

static void doPlayer() {
    if (player != nullptr) {
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
}

static void fireBullet() {
    auto *ball = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(ball, 0, sizeof(Entity));
    stage.ballTail->next = ball;
    stage.ballTail = ball;
    ball->side = SIDE_PLAYER;

    ball->x = player->x;
    ball->y = player->y;
    ball->dx = PLAYER_BULLET_SPEED;
    ball->health = 1;
    ball->texture = ballTexture;
    SDL_QueryTexture(ball->texture, nullptr, nullptr, &ball->w, &ball->h);

    ball->y += (player->h / 2) - (ball->h / 2);

    player->reload = 8;
}

static void doBullets() {
    Entity *prev = &stage.ballHead;

    for (Entity *b = stage.ballHead.next; b != nullptr; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (bulletHitFighter(b) || b->x < -b->w || b->y < -b->h || b->x > SCREEN_WIDTH || b->y > SCREEN_HEIGHT) {
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

static int bulletHitFighter(Entity *b) {
    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        if (e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h)) {
            b->health = 0;
            e->health -= 1;

            return 1;
        }
    }

    return 0;
}

static void doEnemies() {
    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        if (e != player && player != nullptr && --e->reload <= 0) {
            fireAlienBullet(e);
        }
    }
}

static void fireAlienBullet(Entity *e) {
    const auto bullet = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(bullet, 0, sizeof(Entity));
    stage.ballTail->next = bullet;
    stage.ballTail = bullet;

    bullet->x = e->x;
    bullet->y = e->y;
    bullet->health = 1;
    bullet->texture = alienBulletTexture;
    bullet->side = SIDE_ALIEN;
    SDL_QueryTexture(bullet->texture, nullptr, nullptr, &bullet->w, &bullet->h);

    bullet->x += (e->w / 2) - (bullet->w / 2);
    bullet->y += (e->h / 2) - (bullet->h / 2);

    calcSlope(player->x + (player->w / 2), player->y + (player->h / 2), e->x, e->y, &bullet->dx, &bullet->dy);

    bullet->dx *= ALIEN_BULLET_SPEED;
    bullet->dy *= ALIEN_BULLET_SPEED;

    e->reload = (rand() % FPS * 2);
}

static void draw() {
    drawPlayer();
    drawBullets();
    drawFighters();
}

static void drawPlayer() {
    if (player != nullptr) {
        blit(player->texture, player->x, player->y);
    }
}

static void drawBullets() {
    for (const Entity *b = stage.ballHead.next; b != nullptr; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}

static void drawFighters() {
    for (const Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}
