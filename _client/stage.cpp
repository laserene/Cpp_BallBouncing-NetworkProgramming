#include "sys/socket.h"
#include "iostream"

#include "stage.h"
#include "common.h"
#include "draw.h"
#include "defs.h"
#include "util.h"

extern App app;
extern Stage stage;

static void logic(int sock, const fd_set &read_fds);

static void draw();

static void initPlayer();

static void fireBullet();

static void doPlayer(int sock);

static void doFighters();

static void doBullets();

static int bulletHitFighter(Entity *b);

static void drawFighters();

static void drawBullets();

static void spawnEnemies();

static Entity *player;
static SDL_Texture *bulletTexture;
static SDL_Texture *enemyTexture;
static int enemySpawnTimer;

void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    initPlayer();
    bulletTexture = loadTexture(BULLET_TEXTURE);
    enemyTexture = loadTexture(ENEMY_TEXTURE);
    enemySpawnTimer = 0;
}

static void initPlayer() {
    player = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;
    player->side = SIDE_PLAYER;
    player->texture = loadTexture(PLAYER_TEXTURE);
    SDL_QueryTexture(player->texture, nullptr, nullptr, &player->w, &player->h);
}

static void logic(const int sock, const fd_set &read_fds) {
    doPlayer(sock);
    doFighters();
    doBullets();
    spawnEnemies();

    char buffer[BUFFER_SIZE] = {};
    if (FD_ISSET(sock, &read_fds)) {
        memset(buffer, 0, BUFFER_SIZE);
        if (const ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE, 0); bytes_received <= 0) {
            printf("Disconnected from server\n");
            exit(-1);
        }
        handle_server_message(buffer);
    }
}

void handle_server_message(const char *buffer) {
    if (strncmp(buffer, "UPDATE", 6) == 0) {
        int u, d, l, r;
        sscanf(buffer, SERVER_UPDATE, &u, &d, &l, &r);
        player->dy += u;
        player->dy += d;
        player->dx += l;
        player->dx += r;

        player->x += player->dx;
        player->y += player->dy;
    } else {
        std::cout << buffer << std::endl;
    }
}

static void doPlayer(const int sock) {
    player->dx = player->dy = 0;

    if (player->reload > 0) {
        player->reload--;
    }

    char buffer[BUFFER_SIZE] = {};
    if (app.keyboard[SDL_SCANCODE_UP]) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), SEND_MOVE, 1, 0, 0, 0);
        send(sock, buffer, BUFFER_SIZE, 0);
    }

    if (app.keyboard[SDL_SCANCODE_DOWN]) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 1, 0, 0);
        send(sock, buffer, BUFFER_SIZE, 0);
    }

    if (app.keyboard[SDL_SCANCODE_LEFT]) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 0, 1, 0);
        send(sock, buffer, BUFFER_SIZE, 0);
    }

    if (app.keyboard[SDL_SCANCODE_RIGHT]) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 0, 0, 1);
        send(sock, buffer, strlen(buffer), 0);
    }

    if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload == 0) {
        fireBullet();
    }
}

static void doFighters() {
    Entity *prev = &stage.fighterHead;

    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (e != player && (e->x < -e->w || e->health == 0)) {
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

static void fireBullet() {
    auto *bullet = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = player->x;
    bullet->y = player->y;
    bullet->dx = PLAYER_BULLET_SPEED;
    bullet->side = SIDE_PLAYER;
    bullet->health = 1;
    bullet->texture = bulletTexture;
    SDL_QueryTexture(bullet->texture, nullptr, nullptr, &bullet->w, &bullet->h);

    bullet->y += (player->h / 2) - (bullet->h / 2);

    player->reload = 8;
}

static void doBullets() {
    Entity *prev = &stage.bulletHead;

    for (Entity *b = stage.bulletHead.next; b != nullptr; b = b->next) {
        b->x += b->dx;
        b->y += b->dy;

        if (bulletHitFighter(b) || b->x > SCREEN_WIDTH) {
            if (b == stage.bulletTail) {
                stage.bulletTail = prev;
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
            e->health = 0;

            return 1;
        }
    }

    return 0;
}

static void spawnEnemies() {
    if (--enemySpawnTimer <= 0) {
        auto *enemy = static_cast<Entity *>(malloc(sizeof(Entity)));
        memset(enemy, 0, sizeof(Entity));
        stage.fighterTail->next = enemy;
        stage.fighterTail = enemy;

        enemy->x = SCREEN_WIDTH;
        enemy->y = rand() % SCREEN_HEIGHT;
        enemy->texture = enemyTexture;
        enemy->side = SIDE_ALIEN;
        SDL_QueryTexture(enemy->texture, nullptr, nullptr, &enemy->w, &enemy->h);

        enemy->dx = -(2 + (rand() % 4));

        enemySpawnTimer = 30 + (rand() % 60);
    }
}

static void draw() {
    drawFighters();
    drawBullets();
}

static void drawFighters() {
    for (const Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}

static void drawBullets() {
    for (const Entity *b = stage.bulletHead.next; b != nullptr; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}
