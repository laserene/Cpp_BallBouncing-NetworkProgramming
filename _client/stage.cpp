#include "sys/socket.h"
#include "iostream"

#include "stage.h"
#include "common.h"
#include "draw.h"
#include "defs.h"
#include "util.h"
#include "helper.h"

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

static void doExplosions();

static void addExplosions(int x, int y, int num);

static void addDebris(const Entity *e);

static void doDebris();

static int bulletHitFighter(Entity *b);

static void drawFighters();

static void drawBullets();

static void spawnEnemies();

static void fireAlienBullet(Entity *e);

static void doPointsPods();

static void addPointsPod(int x, int y);

static void addDebuff();

static void clipPlayer();

static void resetStage();

static void drawExplosions();

static void drawPointsPods();

static void drawDebris();

static void drawHud();

static void drawHeart(int x, int y);

static void drawBuff(int x, int y);

static void drawDebuff(int x, int y);

// Buff
static void doBuff();

static void apply_widespread();

static void apply_speedup();

static void apply_freeze();

static void apply_luck();

static void apply_heart();

static void apply_refresh();

void reset_widespread();

void reset_speedup();

void reset_luck();

// Debuff
void doDebuff();

void apply_bleeding();

void apply_weak();

void apply_confusion();

void apply_dark();

void apply_chill();

void reset_bleeding();

void reset_weak();

void reset_confusion();

void reset_dark();

void reset_chill();

void reset_debuff();

static Entity *player;
static SDL_Texture *playerTexture;
static SDL_Texture *bulletTexture;
static SDL_Texture *enemyTexture;
static SDL_Texture *enemyBulletTexture;
static SDL_Texture *background;
static SDL_Texture *explosionTexture;

// Buff texture
static SDL_Texture *widespreadTexture;
static SDL_Texture *frozenTexture;
static SDL_Texture *speedupTexture;
static SDL_Texture *luckTexture;
static SDL_Texture *heartTexture;
static SDL_Texture *refreshTexture;

// Debuff texture
static SDL_Texture *bleedingTexture;
static SDL_Texture *weakTexture;
static SDL_Texture *confusionTexture;
static SDL_Texture *darkTexture;
static SDL_Texture *chillTexture;

static int enemySpawnTimer;
static int stageResetTimer;
static int backgroundX;
static int highscore;

Star stars[MAX_STARS] = {};

auto SCORE = "SCORE: %03d";
auto HIGHSCORE = "HIGHSCORE: %03d";

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

    // Invariable texture to biomes
    widespreadTexture = loadTexture(WIDESPREAD_BUFF);
    frozenTexture = loadTexture(FROZEN_BUFF);
    speedupTexture = loadTexture(SPEEDUP_BUFF);
    luckTexture = loadTexture(LUCK_BUFF);
    heartTexture = loadTexture(HEART_BUFF);
    refreshTexture = loadTexture(REFRESH_BUFF);

    bleedingTexture = loadTexture(BLEEDING_DEBUFF);
    weakTexture = loadTexture(WEAK_DEBUFF);
    confusionTexture = loadTexture(CONFUSION_DEBUFF);
    darkTexture = loadTexture(DARK_DEBUFF);
    chillTexture = loadTexture(CHILL_DEBUFF);

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

    while (stage.pointsHead.next) {
        Entity *e = stage.pointsHead.next;
        stage.pointsHead.next = e->next;
        free(e);
    }

    memset(&stage, 0, sizeof(Stage));
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;
    stage.pointsTail = &stage.pointsHead;

    initPlayer();
    initStarfield();

    stage.score = 0;
    enemySpawnTimer = 0;
    stageResetTimer = FPS * 4;
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
    for (auto &star: stars) {
        star.x = rand() % SCREEN_WIDTH;
        star.y = rand() % SCREEN_HEIGHT;
        star.speed = 1 + rand() % 8;
    }
}

static void logic(const int sock, const fd_set &read_fds) {
    doBackground();
    doStarfield();
    doPlayer(sock);

    char buffer[BUFFER_SIZE] = {};
    if (FD_ISSET(sock, &read_fds)) {
        memset(buffer, 0, BUFFER_SIZE);
        if (const ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE, 0); bytes_received <= 0) {
            printf("Disconnected from server\n");
            exit(-1);
        }
        handle_server_message(buffer);
    }

    doEnemies();
    doFighters();
    doBullets();
    doExplosions();
    doDebris();
    doPointsPods();
    spawnEnemies();
    doBuff();
    doDebuff();

    clipPlayer();

    if (player == nullptr && --stageResetTimer <= 0) {
        resetStage();
    }
}

void handle_server_message(const char *buffer) {
    if (player != nullptr) {
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

static void doPointsPods() {
    // Looping all current point pods to check
    Entity *prev = &stage.pointsHead;
    for (Entity *e = stage.pointsHead.next; e != nullptr; e = e->next) {
        // Reflect if collides mit edge
        if (e->x < 0) {
            e->x = 0;
            e->dx = -e->dx;
        }

        // Accounting for entity height
        if (e->x + e->w > SCREEN_WIDTH) {
            e->x = SCREEN_WIDTH - e->w;
            e->dx = -e->dx;
        }

        // Reflect if collides mit edge
        if (e->y < 0) {
            e->y = 0;
            e->dy = -e->dy;
        }

        // Accounting for entity height
        if (e->y + e->h > SCREEN_HEIGHT) {
            e->y = SCREEN_HEIGHT - e->h;
            e->dy = -e->dy;
        }

        // Moving
        e->x += e->dx;
        e->y += e->dy;

        // Colliding mit player
        if (player != nullptr && collision(e->x, e->y, e->w, e->h, player->x, player->y, player->w, player->h)) {
            e->health = 0;

            if (const int pod_id = e->pod_id; pod_id > 0) {
                switch (pod_id) {
                    case WIDESPREAD:
                    case SPEEDUP:
                    case LUCK:
                        for (auto &i: stage.buffList) {
                            const int buff_id = i.id;

                            // Duplicated buff --> refresh buff
                            if (pod_id == buff_id) {
                                i.health = 15 * FPS;
                                break;
                            }

                            /*
                             * Non-duplicated existing buff
                             * Check if duplicated buff --> refresh buff. Then break the loop since buff consumed
                             */
                            if (buff_id != 0) continue;

                            i.id = pod_id;
                            i.texture = e->texture;
                            i.health = 15 * FPS;
                            break;
                        }
                        break;
                    case FROZEN:
                        apply_freeze();
                        break;
                    case HEART:
                        apply_heart();
                        break;
                    case REFRESH:
                        // TODO: Refresh stat and remove all elements in stage.debuffList
                        apply_refresh();
                        break;
                    default:
                        break;
                }
            } else if (pod_id < 0) {
                for (auto &i: stage.debuffList) {
                    const int debuff_id = i.id;

                    // Duplicated buff --> refresh buff
                    if (pod_id == debuff_id) {
                        i.health = 15 * FPS;
                        break;
                    }

                    /*
                     * Non-duplicated existing debuff
                     * Check if duplicated debuff --> refresh debuff. Then break the loop since debuff consumed
                     */
                    if (debuff_id != 0) continue;

                    i.id = pod_id;
                    i.texture = e->texture;
                    i.health = 15 * FPS;
                    break;
                }
            }

            stage.score++;
            highscore = MAX(stage.score, highscore);
        }

        // Pods exist in 15s at max
        if (--e->health <= 0) {
            if (e == stage.pointsTail) {
                stage.pointsTail = prev;
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
            e->health -= 1;

            if (e->health <= 0) {
                addExplosions(e->x, e->y, 32);
                addDebris(e);
                if (e->side == SIDE_ALIEN) {
                    // Generte buff, debuff
                    if (getRandomNumber(1, 100) <= BUFF_RATE) addPointsPod(e->x + e->w / 2, e->y + e->h / 2);
                    else if (getRandomNumber(1, 100) <= DEBUFF_RATE) addDebuff();
                }
            }

            stage.score++;
            highscore = MAX(stage.score, highscore);

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

void doBackground() {
    if (--backgroundX < -SCREEN_WIDTH) {
        backgroundX = 0;
    }
}

void doStarfield() {
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

static void addPointsPod(const int x, const int y) {
    const auto e = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(e, 0, sizeof(Entity));

    e->x = x;
    e->y = y;
    e->dx = -(rand() % 5);
    e->dy = (rand() % 5) - (rand() % 5);

    if (e->dx == 0) e->dx -= 1;
    if (e->dy == 0) e->dy += 1;

    e->health = FPS * 15;

    int pod_id = getRandomNumber(1, 100) % NUM_BUFF + 1;
    e->pod_id = pod_id;
    switch (pod_id) {
        case WIDESPREAD:
            e->texture = widespreadTexture;
            break;
        case FROZEN:
            e->texture = frozenTexture;
            break;
        case SPEEDUP:
            e->texture = speedupTexture;
            break;
        case LUCK:
            e->texture = luckTexture;
            break;
        case HEART:
            e->texture = heartTexture;
            break;
        case REFRESH:
            e->texture = refreshTexture;
            break;
        default:
            break;
    }

    SDL_QueryTexture(e->texture, nullptr, nullptr, &e->w, &e->h);
    e->x -= e->w / 2;
    e->y -= e->h / 2;

    stage.pointsTail->next = e;
    stage.pointsTail->next = e;
    stage.pointsTail = e;
}

static void addDebuff() {
    const auto p = static_cast<Pod *>(malloc(sizeof(Pod)));
    memset(p, 0, sizeof(Pod));

    p->id = getRandomNumber(1, 100) % NUM_DEBUFF;;

    switch (p->id) {
        case BLEEDING:
            p->texture = bleedingTexture;
            break;
        case WEAK:
            p->texture = weakTexture;
            break;
        case CONFUSION:
            p->texture = confusionTexture;
            break;
        case DARK:
            p->texture = darkTexture;
            break;
        case CHILL:
            p->texture = chillTexture;
            break;
        default:
            break;
    }

    for (auto &i: stage.debuffList) {
        const int debuff_id = i.id;

        // Duplicated buff --> refresh buff
        if (p->id == debuff_id) {
            i.health = 15 * FPS;
            break;
        }

        /*
         * Non-duplicated existing buff
         * Check if duplicated buff --> refresh buff. Then break the loop since buff consumed
         */
        if (debuff_id != 0) continue;

        i.id = p->id;
        i.texture = p->texture;
        i.health = 15 * FPS;
        break;
    }

    free(p);
}

static void draw() {
    drawBackground();
    drawStarfield();
    drawFighters();
    drawBullets();
    drawPointsPods();
    drawDebris();
    drawExplosions();
    drawHud();
}

void drawBackground() {
    SDL_Rect dest;

    for (int x = backgroundX; x < SCREEN_WIDTH; x += SCREEN_WIDTH) {
        dest.x = x;
        dest.y = 0;
        dest.w = SCREEN_WIDTH;
        dest.h = SCREEN_HEIGHT;

        SDL_RenderCopy(app.renderer, background, nullptr, &dest);
    }
}

void drawStarfield() {
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

static void drawPointsPods() {
    for (const Entity *e = stage.pointsHead.next; e != nullptr; e = e->next) {
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

static void drawHud() {
    drawText(10, 10, 255, 255, 255, SCORE, stage.score);
    drawHeart(10, 40);
    drawDebuff(200, 6);
    drawBuff(200, 40);

    if (stage.score > 0 && stage.score == highscore) {
        drawText(1336, 10, 0, 255, 0, HIGHSCORE, highscore);
    } else {
        drawText(1336, 10, 255, 255, 255, HIGHSCORE, highscore);
    }
}

static void drawHeart(const int x, const int y) {
    if (heartTexture != nullptr && player != nullptr) {
        for (int i = 0; i < player->health; i++) {
            if (i != 9) {
                const int heartX = x + i * 20;
                blit(heartTexture, heartX, y);
            } else {
                blit(heartTexture, x, y + 20);
            }
        }
    }
}

static void drawDebuff(const int x, const int y) {
    int count_debuff = 0;
    for (auto &i: stage.debuffList) {
        if (i.id != 0) {
            const int debuffX = x + count_debuff * 34;
            count_debuff += 1;
            blit(i.texture, debuffX, y);
        }
    }
}

static void drawBuff(const int x, const int y) {
    int count_buff = 0;
    for (auto &i: stage.buffList) {
        if (i.id != 0) {
            const int buffX = x + count_buff * 34;
            count_buff += 1;
            blit(i.texture, buffX, y);
        }
    }
}

// Buff
static void doBuff() {
    for (auto &i: stage.buffList) {
        if (i.id == 0) continue;
        i.health -= 1;
        switch (i.id) {
            case WIDESPREAD:
                if (i.health <= 0) {
                    i.id = 0;
                    reset_widespread();
                } else apply_widespread();
                break;
            case SPEEDUP:
                if (i.health <= 0) {
                    i.id = 0;
                    reset_speedup();
                } else apply_speedup();
                break;
            case LUCK:
                if (i.health <= 0) {
                    i.id = 0;
                    reset_luck();
                } else apply_luck();
                break;
            default:
                break;
        }
    }
}

static void apply_widespread() {
}

static void apply_freeze() {
    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        e->dx = 0;
        e->dy = 0;
    }
}

void apply_speedup() {
}

void apply_luck() {
}

static void apply_heart() {
    if (player->health < 10) player->health += 1;
}

static void apply_refresh() {
}

void reset_widespread() {
}

void reset_speedup() {
}

void reset_luck() {
}

// Debuff
void doDebuff() {
    for (auto &i: stage.debuffList) {
        if (i.id == 0) continue;
        i.health -= 1;
        switch (i.id - 1) {
            case BLEEDING:
                if (i.health == 0) {
                    i.id = 0;
                    reset_bleeding();
                } else apply_bleeding();
                break;
            case WEAK:
                if (i.health == 0) {
                    i.id = 0;
                    reset_weak();
                } else apply_weak();
                break;
            case CONFUSION:
                if (i.health == 0) {
                    i.id = 0;
                    reset_confusion();
                } else apply_confusion();
                break;
            case DARK:
                if (i.health == 0) {
                    i.id = 0;
                    reset_dark();
                } else apply_dark();
                break;
            case CHILL:
                if (i.health == 0) {
                    i.id = 0;
                    reset_chill();
                } else apply_chill();
                break;
            default:
                break;
        }
    }
}


void apply_bleeding() {
}

void apply_weak() {
}

void apply_confusion() {
}

void apply_dark() {
}

void apply_chill() {
}

void reset_bleeding() {
}

void reset_weak() {
}

void reset_confusion() {
}

void reset_dark() {
}

void reset_chill() {
}

void reset_debuff() {
    memset(stage.debuffList, 0, sizeof(Pod) * NUM_DEBUFF);
    reset_bleeding();
    reset_weak();
    reset_confusion();
    reset_dark();
    // if (stat.player_delta > 0) reset_chilled();
}
