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

static void initStarfield();

static void fireBullet();

static void doPlayer(int sock);

static void doFighters();

static void doEnemies();

static void doBullets();

static void doBackground();

static void doStarfield();

static void doExplosions();

static void addExplosions(int x, int y, int num);

static void addDebris(const Entity *e);

static void doDebris();

static int bulletHitFighter(Entity *b);

static void drawFighters();

static void drawBullets();

static void spawnEnemies();

static void fireAlienBullet(Entity *e);

static void clipPlayer();

static void resetStage();

static void drawExplosions();

static void drawBackground();

static void drawStarfield();

static void drawDebris();

static Entity *player;
static SDL_Texture *playerTexture;
static SDL_Texture *bulletTexture;
static SDL_Texture *enemyTexture;
static SDL_Texture *enemyBulletTexture;
static SDL_Texture *background;
static SDL_Texture *explosionTexture;
static int enemySpawnTimer;
static int stageResetTimer;
static int backgroundX;

Star stars[MAX_STARS] = {};

void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;

    playerTexture = loadTexture(PLAYER_TEXTURE);
    bulletTexture = loadTexture(BULLET_TEXTURE);
    enemyTexture = loadTexture(ENEMY_TEXTURE);
    enemyBulletTexture = loadTexture(ENEMY_BULLET_TEXTURE);
    background = loadTexture(BACKGROUND_TEXTURE);
    explosionTexture = loadTexture(EXPLOSION_TEXTURE);

    resetStage();
}

static void resetStage() {
    while (stage.fighterHead.next) {
        Entity *e = stage.fighterHead.next;
        stage.fighterHead.next = e->next;
        free(e);
    }

    while (stage.bulletHead.next) {
        Entity *e = stage.bulletHead.next;
        stage.bulletHead.next = e->next;
        free(e);
    }

    while (stage.explosionHead.next) {
        Explosion *ex = stage.explosionHead.next;
        stage.explosionHead.next = ex->next;
        free(ex);
    }

    while (stage.debrisHead.next) {
        Debris *d = stage.debrisHead.next;
        stage.debrisHead.next = d->next;
        free(d);
    }

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;

    initPlayer();
    initStarfield();

    enemySpawnTimer = 0;
    stageResetTimer = FPS * 3;
}

static void initPlayer() {
    player = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(player, 0, sizeof(Entity));
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;
    player->health = 3;
    player->side = SIDE_PLAYER;
    player->texture = playerTexture;
    SDL_QueryTexture(player->texture, nullptr, nullptr, &player->w, &player->h);
}

static void initStarfield() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
        stars[i].speed = 1 + rand() % 8;
    }
}

static void logic(const int sock, const fd_set &read_fds) {
    doBackground();
    doStarfield();
    doPlayer(sock);
    doEnemies();
    doFighters();
    doBullets();
    doExplosions();
    doDebris();
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

    clipPlayer();

    if (player == nullptr && --stageResetTimer <= 0) {
        resetStage();
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
    if (player != nullptr) {
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

static void doEnemies() {
    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        if (e != player && player != nullptr && --e->reload <= 0) {
            fireAlienBullet(e);
        }
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

        if (bulletHitFighter(b) || b->x < -b->w || b->y < -b->h || b->x > SCREEN_WIDTH || b->y > SCREEN_HEIGHT) {
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

            addExplosions(e->x, e->y, 32);

            addDebris(e);

            return 1;
        }
    }

    return 0;
}

static void fireAlienBullet(Entity *e) {
    auto *bullet = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(bullet, 0, sizeof(Entity));
    stage.bulletTail->next = bullet;
    stage.bulletTail = bullet;

    bullet->x = e->x;
    bullet->y = e->y;
    bullet->health = 1;
    bullet->texture = enemyBulletTexture;
    bullet->side = SIDE_ALIEN;
    SDL_QueryTexture(bullet->texture, nullptr, nullptr, &bullet->w, &bullet->h);

    bullet->x += (e->w / 2) - (bullet->w / 2);
    bullet->y += (e->h / 2) - (bullet->h / 2);

    calcSlope(player->x + (player->w / 2), player->y + (player->h / 2), e->x, e->y, &bullet->dx, &bullet->dy);

    bullet->dx *= ALIEN_BULLET_SPEED;
    bullet->dy *= ALIEN_BULLET_SPEED;

    e->reload = (rand() % FPS * 2);
}

static void spawnEnemies() {
    if (--enemySpawnTimer <= 0) {
        auto *enemy = static_cast<Entity *>(malloc(sizeof(Entity)));
        memset(enemy, 0, sizeof(Entity));
        stage.fighterTail->next = enemy;
        stage.fighterTail = enemy;

        enemy->x = SCREEN_WIDTH;
        enemy->y = rand() % SCREEN_HEIGHT;
        enemy->health = 1;
        enemy->texture = enemyTexture;
        enemy->side = SIDE_ALIEN;
        SDL_QueryTexture(enemy->texture, nullptr, nullptr, &enemy->w, &enemy->h);

        enemy->dx = -(2 + (rand() % 4));
        enemy->reload = FPS * (1 + (rand() % 3));
        enemySpawnTimer = 30 + (rand() % 60);
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

        if (player->x > SCREEN_WIDTH / 2) {
            player->x = SCREEN_WIDTH / 2;
        }

        if (player->y > SCREEN_HEIGHT - player->h) {
            player->y = SCREEN_HEIGHT - player->h;
        }
    }
}

static void doBackground() {
    if (--backgroundX < -SCREEN_WIDTH) {
        backgroundX = 0;
    }
}

static void doStarfield() {
    for (auto &star: stars) {
        star.x -= star.speed;

        if (star.x < 0) {
            star.x = SCREEN_WIDTH + star.x;
        }
    }
}

static void doExplosions() {
    Explosion *prev = &stage.explosionHead;

    for (Explosion *e = stage.explosionHead.next; e != nullptr; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (--e->a <= 0) {
            if (e == stage.explosionTail) {
                stage.explosionTail = prev;
            }

            prev->next = e->next;
            free(e);
            e = prev;
        }

        prev = e;
    }
}

static void doDebris() {
    Debris *prev = &stage.debrisHead;

    for (Debris *d = stage.debrisHead.next; d != nullptr; d = d->next) {
        d->x += d->dx;
        d->y += d->dy;

        d->dy += 0.5;

        if (--d->life <= 0) {
            if (d == stage.debrisTail) {
                stage.debrisTail = prev;
            }

            prev->next = d->next;
            free(d);
            d = prev;
        }

        prev = d;
    }
}

static void addExplosions(const int x, const int y, const int num) {
    for (int i = 0; i < num; i++) {
        auto *e = static_cast<Explosion *>(malloc(sizeof(Explosion)));
        memset(e, 0, sizeof(Explosion));
        stage.explosionTail->next = e;
        stage.explosionTail = e;

        e->x = x + (rand() % 32) - (rand() % 32);
        e->y = y + (rand() % 32) - (rand() % 32);
        e->dx = (rand() % 10) - (rand() % 10);
        e->dy = (rand() % 10) - (rand() % 10);

        e->dx /= 10;
        e->dy /= 10;

        switch (rand() % 4) {
            case 0:
                e->r = 255;
                break;

            case 1:
                e->r = 255;
                e->g = 128;
                break;

            case 2:
                e->r = 255;
                e->g = 255;
                break;

            default:
                e->r = 255;
                e->g = 255;
                e->b = 255;
                break;
        }

        e->a = rand() % FPS * 3;
    }
}

static void addDebris(const Entity *e) {
    const int w = e->w / 2;
    const int h = e->h / 2;

    for (int y = 0; y <= h; y += h) {
        for (int x = 0; x <= w; x += w) {
            auto *d = static_cast<Debris *>(malloc(sizeof(Debris)));
            memset(d, 0, sizeof(Debris));
            stage.debrisTail->next = d;
            stage.debrisTail = d;

            d->x = e->x + e->w / 2;
            d->y = e->y + e->h / 2;
            d->dx = (rand() % 5) - (rand() % 5);
            d->dy = -(5 + (rand() % 12));
            d->life = FPS * 2;
            d->texture = e->texture;

            d->rect.x = x;
            d->rect.y = y;
            d->rect.w = w;
            d->rect.h = h;
        }
    }
}

static void draw() {
    drawBackground();
    drawStarfield();
    drawFighters();
    drawBullets();
    drawDebris();
    drawExplosions();
}

static void drawBackground() {
    SDL_Rect dest;

    for (int x = backgroundX; x < SCREEN_WIDTH; x += SCREEN_WIDTH) {
        dest.x = x;
        dest.y = 0;
        dest.w = SCREEN_WIDTH;
        dest.h = SCREEN_HEIGHT;

        SDL_RenderCopy(app.renderer, background, nullptr, &dest);
    }
}

static void drawStarfield() {
    for (auto &star: stars) {
        const int c = 32 * star.speed;

        SDL_SetRenderDrawColor(app.renderer, c, c, c, 255);

        SDL_RenderDrawLine(app.renderer, star.x, star.y, star.x + 3, star.y);
    }
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

static void drawDebris() {
    for (const Debris *d = stage.debrisHead.next; d != nullptr; d = d->next) {
        blitRect(d->texture, &d->rect, d->x, d->y);
    }
}

static void drawExplosions() {
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(explosionTexture, SDL_BLENDMODE_ADD);

    for (const Explosion *e = stage.explosionHead.next; e != nullptr; e = e->next) {
        SDL_SetTextureColorMod(explosionTexture, e->r, e->g, e->b);
        SDL_SetTextureAlphaMod(explosionTexture, e->a);

        blit(explosionTexture, e->x, e->y);
    }

    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}
