#include "sys/socket.h"
#include "iostream"

#include "stage.h"
#include "common.h"
#include "draw.h"
#include "defs.h"

extern App app;
extern Stage stage;

static void logic(int sock, const fd_set &read_fds);

static void draw();

static void initPlayer();

static void fireBullet();

static void doPlayer(int sock);

static void doBullets();

static void drawPlayer();

static void drawBullets();

static Entity *player;
static SDL_Texture *bulletTexture;

void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    initPlayer();
    bulletTexture = loadTexture(BULLET_TEXTURE);
}

static void initPlayer() {
    player = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;
    player->texture = loadTexture(PLAYER_TEXTURE);
    SDL_QueryTexture(player->texture, nullptr, nullptr, &player->w, &player->h);
}

static void logic(const int sock, const fd_set &read_fds) {
    doPlayer(sock);
    doBullets();

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
        player->dy += l;
        player->dy += r;
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

    player->x += player->dx;
    player->y += player->dy;
}

static void fireBullet() {
    auto *bullet = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = player->x;
    bullet->y = player->y;
    bullet->dx = PLAYER_BULLET_SPEED;
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

        if (b->x > SCREEN_WIDTH) {
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

static void draw() {
    drawPlayer();
    drawBullets();
}

static void drawPlayer() {
    blit(player->texture, player->x, player->y);
}

static void drawBullets() {
    for (const Entity *b = stage.bulletHead.next; b != nullptr; b = b->next) {
        blit(b->texture, b->x, b->y);
    }
}
