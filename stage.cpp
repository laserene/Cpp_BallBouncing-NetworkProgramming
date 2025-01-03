#include "cstdlib"
#include "helper.h"
#include "stage.h"

#include "common.h"
#include "draw.h"
#include "util.h"
#include "text.h"
#include "defs.h"

Entity *player;
SDL_Texture *playerTexture;
SDL_Texture *alienBulletTexture;
SDL_Texture *ballTexture;
SDL_Texture *enemyTexture;
SDL_Texture *background;
SDL_Texture *explosionTexture;
SDL_Texture *pointsTexture;

// Buff texture
SDL_Texture *addBulletTexture;
SDL_Texture *freezeTexture;
SDL_Texture *speedUpTexture;
SDL_Texture *luckTexture;
SDL_Texture *heartTexture;
SDL_Texture *refreshTexture;

// Debuff texture
SDL_Texture *bleedingTexture;
SDL_Texture *weakTexture;
SDL_Texture *confusionTexture;
SDL_Texture *darknessTexture;
SDL_Texture *chilledTexture;

inline Stage stage;
inline Stat stat;
int enemySpawnTimer;
int stageResetTimer;
int backgroundX;
int highscore = 0;

Star stars[MAX_STARS];

// Init
void initStage() {
    app.delegate.logic = logic;
    app.delegate.draw = draw;

    memset(&stage, 0, sizeof(Stage));
    memset(&stat, 0, sizeof(Stat));
    stage.fighterTail = &stage.fighterHead;
    stage.ballTail = &stage.ballHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;
    stage.pointsTail = &stage.pointsHead;
    stat.alpha = 255;

    initPlayer();

    ballTexture = loadTexture(BALL_PATH);
    enemyTexture = loadTexture(ENEMY_PATH);
    alienBulletTexture = loadTexture("../gfx/enemy_bomb.png");
    playerTexture = loadTexture("../gfx/clothier.png");
    background = loadTexture("../map/space.jpg");
    explosionTexture = loadTexture("../gfx/star.png");

    addBulletTexture = loadTexture("../buff/b1.png");
    freezeTexture = loadTexture("../buff/b2.png");
    speedUpTexture = loadTexture("../buff/b3.png");
    luckTexture = loadTexture("../buff/b4.png");
    heartTexture = loadTexture("../buff/b5.png");
    refreshTexture = loadTexture("../buff/b6.png");

    bleedingTexture = loadTexture("../debuff/db1.png");
    weakTexture = loadTexture("../debuff/db2.png");
    confusionTexture = loadTexture("../debuff/db3.png");
    darknessTexture = loadTexture("../debuff/db4.png");
    chilledTexture = loadTexture("../debuff/db5.png");

    const int buff_id = rand() % 8 + 1;
    char result[20];
    snprintf(result, sizeof(result), "../buff/b%d.png", buff_id);
    pointsTexture = loadTexture(result);

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
        e = stage.pointsHead.next;
        stage.pointsHead.next = e->next;
        free(e);
    }

    memset(&stage, 0, sizeof(Stage));
    memset(&stat, 0, sizeof(Stat));
    stage.fighterTail = &stage.fighterHead;
    stage.ballTail = &stage.ballHead;
    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;
    stage.pointsTail = &stage.pointsHead;
    stat.alpha = 255;

    initPlayer();
    initStarfield();

    enemySpawnTimer = 0;
    stage.score = 0;
    stageResetTimer = FPS * 3;
}

static void initStarfield() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
        stars[i].speed = 1 + rand() % 8;
    }
}

static void logic() {
    doBackground();
    doStarfield();

    doDebuff();
    doPlayer();
    doFighters();
    doBullets();
    spawnEnemies();
    doEnemies();
    clipPlayer();

    if (player == nullptr && --stageResetTimer <= 0) {
        resetStage();
    }

    doExplosions();
    doDebris();
    doPointsPods();
}

static void doBackground() {
    if (--backgroundX < -SCREEN_WIDTH) {
        backgroundX = 0;
    }
}

static void doStarfield() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x -= stars[i].speed;

        if (stars[i].x < 0) {
            stars[i].x = SCREEN_WIDTH + stars[i].x;
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

static void doFighters() {
    Entity *prev = &stage.fighterHead;

    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        e->x += e->dx;
        e->y += e->dy;

        if (e != player && e->x < -e->w) {
            e->health = 0;
        }

        if (e->health <= 0) {
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
            player->dy = -PLAYER_SPEED + stat.player_delta;
        }

        if (app.keyboard[SDL_SCANCODE_DOWN]) {
            player->dy = PLAYER_SPEED - stat.player_delta;
        }

        if (app.keyboard[SDL_SCANCODE_LEFT]) {
            player->dx = -PLAYER_SPEED + stat.player_delta;
        }

        if (app.keyboard[SDL_SCANCODE_RIGHT]) {
            player->dx = PLAYER_SPEED - stat.player_delta;
        }

        if (app.keyboard[SDL_SCANCODE_LCTRL] && player->reload == 0) {
            if (!stat.enforced_bullet) fireBullet();
            else fireEnforcedBullet();
        }

        player->x += player->dx + stat.player_delta_dx;
        player->y += player->dy + stat.player_delta_dx;
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
    ball->health = 1 + stat.player_delta_bullet;
    ball->texture = ballTexture;
    SDL_QueryTexture(ball->texture, nullptr, nullptr, &ball->w, &ball->h);

    ball->y += (player->h / 2) - (ball->h / 2);

    player->reload = 8;
}

static void fireEnforcedBullet() {
    auto *ball1 = static_cast<Entity *>(malloc(sizeof(Entity)));
    auto *ball2 = static_cast<Entity *>(malloc(sizeof(Entity)));
    auto *ball3 = static_cast<Entity *>(malloc(sizeof(Entity)));

    memset(ball1, 0, sizeof(Entity));
    memset(ball2, 0, sizeof(Entity));
    memset(ball3, 0, sizeof(Entity));

    stage.ballTail->next = ball1;
    stage.ballTail = ball1;
    stage.ballTail->next = ball2;
    stage.ballTail = ball2;
    stage.ballTail->next = ball3;
    stage.ballTail = ball3;

    ball1->side = SIDE_PLAYER;
    ball2->side = SIDE_PLAYER;
    ball3->side = SIDE_PLAYER;

    ball1->x = player->x;
    ball2->x = player->x;
    ball3->x = player->x;

    ball1->y = player->y;
    ball2->y = player->y;
    ball3->y = player->y;

    ball1->dx = PLAYER_BULLET_SPEED;
    ball1->dy = PLAYER_BULLET_SPEED;

    ball2->dx = PLAYER_BULLET_SPEED;

    ball3->dx = PLAYER_BULLET_SPEED;
    ball3->dy = -PLAYER_BULLET_SPEED;

    ball1->health = 1 + stat.player_delta_bullet;
    ball2->health = 1 + stat.player_delta_bullet;
    ball3->health = 1 + stat.player_delta_bullet;

    ball1->texture = ballTexture;
    ball2->texture = ballTexture;
    ball3->texture = ballTexture;

    SDL_QueryTexture(ball1->texture, nullptr, nullptr, &ball1->w, &ball1->h);
    SDL_QueryTexture(ball2->texture, nullptr, nullptr, &ball2->w, &ball2->h);
    SDL_QueryTexture(ball3->texture, nullptr, nullptr, &ball3->w, &ball3->h);

    ball1->y += (player->h / 2) - (ball1->h / 2);
    ball2->y += (player->h / 2) - (ball2->h / 2);
    ball3->y += (player->h / 2) - (ball3->h / 2);

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
            e->health -= b->health;
            b->health = 0;

            if (e->health <= 0) {
                addExplosions(e->x, e->y, 1);
                addDebris(e);

                if (e->side == SIDE_ALIEN) {
                    addPointsPod(e->x + e->w / 2, e->y + e->h / 2);
                }
            }

            if (b->side == SIDE_PLAYER) {
                stage.score++;
                highscore = MAX(stage.score, highscore);
            }

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
    bullet->health = 1 + stat.enemy_delta_bullet;
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

static void addDebris(Entity *e) {
    int w = e->w / 2;
    int h = e->h / 2;

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

static void doPointsPods() {
    Entity *prev = &stage.pointsHead;

    for (Entity *e = stage.pointsHead.next; e != nullptr; e = e->next) {
        if (e->x < 0) {
            e->x = 0;
            e->dx = -e->dx;
        }

        if (e->x + e->w > SCREEN_WIDTH) {
            e->x = SCREEN_WIDTH - e->w;
            e->dx = -e->dx;
        }

        if (e->y < 0) {
            e->y = 0;
            e->dy = -e->dy;
        }

        if (e->y + e->h > SCREEN_HEIGHT) {
            e->y = SCREEN_HEIGHT - e->h;
            e->dy = -e->dy;
        }

        e->x += e->dx;
        e->y += e->dy;

        if (player != nullptr && collision(e->x, e->y, e->w, e->h, player->x, player->y, player->w, player->h)) {
            e->health = 0;

            // TODO: Add buff effect
            int e_buff_type = e->buff_type;
            switch (e_buff_type - 1) {
                case ENFORCED_BULLET:
                    for (int i = 0; i < NUM_BUFF; i++) {
                        int id = stage.buffList[i].id;

                        // Duplicated buff
                        if (e_buff_type == id) break;

                        // Non-duplicated existing buff
                        if (id != 0) continue;

                        stage.buffList[i].id = e_buff_type;
                        stage.buffList[i].texture = e->texture;
                        break;
                    }
                    stat.enforced_bullet = 1;
                    break;
                case FREEZE:
                    for (Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
                        e->dx = 0;
                        e->dy = 0;
                    }
                    break;
                case SPEEDUP:
                    for (int i = 0; i < NUM_BUFF; i++) {
                        int id = stage.buffList[i].id;

                        // Duplicated buff
                        if (e_buff_type == id) break;

                        // Non-duplicated existing buff
                        if (id != 0) continue;

                        stage.buffList[i].id = e_buff_type;
                        stage.buffList[i].texture = e->texture;
                        break;
                    }
                    stat.player_delta = -1;
                    break;
                case LUCK:
                    for (int i = 0; i < NUM_BUFF; i++) {
                        int id = stage.buffList[i].id;

                        // Duplicated buff
                        if (e_buff_type == id) break;

                        // Non-duplicated existing buff
                        if (id != 0) continue;

                        stage.buffList[i].id = e_buff_type;
                        stage.buffList[i].texture = e->texture;
                        break;
                    }
                    stat.player_delta_luck = 5;
                    break;
                case HEART:
                    if (player->health < 10) player->health += 1;
                    printf("heart");
                    break;
                case REFRESH:
                    // TODO: Refresh stat and remove all elements in stage.debuffList
                    reset_debuff();
                    break;
            }

            stage.score++;
            highscore = MAX(stage.score, highscore);
        }

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

static void addPointsPod(int x, int y) {
    auto *e = static_cast<Entity *>(malloc(sizeof(Entity)));
    memset(e, 0, sizeof(Entity));
    stage.pointsTail->next = e;
    stage.pointsTail = e;

    e->x = x;
    e->y = y;
    e->dx = -(rand() % 5);
    e->dy = (rand() % 5) - (rand() % 5);

    if (e->dx == 0 || e->dy == 0) {
        e->dx = 2;
        e->dy = 1;
    }

    e->health = FPS * 10;

    int id;
    if (getRandomNumber(1, 100) < BUFF_THRESHOLD + stat.player_delta_luck + 100) {
        id = rand() % NUM_BUFF;

        switch (id) {
            case ENFORCED_BULLET:
                e->texture = addBulletTexture;
                e->buff_type = id + 1;
                break;
            case FREEZE:
                e->texture = freezeTexture;
                e->buff_type = id + 1;
                break;
            case SPEEDUP:
                e->texture = speedUpTexture;
                e->buff_type = id + 1;
                break;
            case LUCK:
                e->texture = luckTexture;
                e->buff_type = id + 1;
                break;
            case HEART:
                e->texture = heartTexture;
                e->buff_type = id + 1;
                break;
            case REFRESH:
                e->texture = refreshTexture;
                e->buff_type = id + 1;
                break;
        }
        SDL_QueryTexture(e->texture, nullptr, nullptr, &e->w, &e->h);
        e->x -= e->w / 2;
        e->y -= e->h / 2;
    } else if (getRandomNumber(1, 100) < DEBUFF_THRESHOLD - stat.player_delta_luck) {
        // Debuffs are consumed instantly
        id = rand() % NUM_DEBUFF;
        consumeDebuff(id);
    }
}

static void consumeDebuff(const int id) {
    int i;
    for (i = 0; i < NUM_DEBUFF; i++) {
        // Duplicated debuff
        if (stage.debuffList[i].id == id + 1) {
            return;
        }

        if (stage.debuffList[i].id == 0) {
            stage.debuffList[i].id = id + 1;
            stage.debuffList[i].time_to_live = 10 * FPS;
            break;
        }
    }

    // Handle out-of-range index
    if (i == NUM_DEBUFF) i = NUM_DEBUFF - 1;

    switch (stage.debuffList[i].id - 1) {
        case BLEEDING:
            stage.debuffList[i].texture = bleedingTexture;
            break;
        case WEAK:
            stage.debuffList[i].texture = weakTexture;
            break;
        case CONFUSION:
            stage.debuffList[i].texture = confusionTexture;
            break;
        case DARKNESS:
            stage.debuffList[i].texture = darknessTexture;
            break;
        case CHILLED:
            stage.debuffList[i].texture = chilledTexture;
            break;
        default:
            break;
    }
}

// Draw
static void draw() {
    drawBackground();
    drawStarfield();
    drawPointsPods();

    drawPlayer();
    drawBullets();
    drawFighters();

    drawDebris();
    drawExplosions();

    drawHud();
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

    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND); // Enable transparency
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255 - stat.alpha); // 50% darkness
    SDL_Rect darkOverlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}; // Full screen
    SDL_RenderFillRect(app.renderer, &darkOverlay);
}

static void drawStarfield() {
    for (int i = 0; i < MAX_STARS; i++) {
        const int c = 32 * stars[i].speed;

        SDL_SetRenderDrawColor(app.renderer, c, c, c, 255);
        SDL_RenderDrawLine(app.renderer, stars[i].x, stars[i].y, stars[i].x + 3, stars[i].y);
    }
}

static void drawPointsPods() {
    for (const Entity *e = stage.pointsHead.next; e != nullptr; e = e->next) {
        blit(e->texture, e->x, e->y);
    }
}

static void drawPlayer() {
    if (player != nullptr) {
        blit(player->texture, player->x, player->y);
    }
}

static void drawBullets() {
    for (const Entity *b = stage.ballHead.next; b != nullptr; b = b->next) {
        if (b->side == SIDE_ALIEN) {
            SDL_SetTextureAlphaMod(b->texture, stat.alpha);
        }
        SDL_Rect dest;
        dest.x = b->x;
        dest.y = b->y;
        SDL_QueryTexture(b->texture, nullptr, nullptr, &dest.w, &dest.h);
        SDL_RenderCopy(app.renderer, b->texture, nullptr, &dest);
    }
}

static void drawFighters() {
    for (const Entity *e = stage.fighterHead.next; e != nullptr; e = e->next) {
        blit(e->texture, e->x, e->y);
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
    drawText(10, 10, 255, 255, 255, "SCORE: %03d", stage.score);
    drawDebuff(200, 6);
    drawHeart(10, 40);
    drawBuff(200, 40);

    if (stage.score > 0 && stage.score == highscore) {
        drawText(960, 10, 0, 255, 0, "HIGH SCORE: %03d", highscore);
    } else {
        drawText(960, 10, 255, 255, 255, "HIGH SCORE: %03d", highscore);
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
    for (int i = 0; i < NUM_DEBUFF; i++) {
        if (stage.debuffList[i].id != -1) {
            const int debuffX = x + count_debuff * 34;
            count_debuff += 1;
            blit(stage.debuffList[i].texture, debuffX, y);
        }
    }
}

static void drawBuff(int x, int y) {
    int count_buff = 0;
    for (int i = 0; i < NUM_BUFF; i++) {
        if (stage.buffList[i].id != 0) {
            const int buffX = x + count_buff * 34;
            count_buff += 1;
            blit(stage.buffList[i].texture, buffX, y);
        }
    }
}

// Debuff
void doDebuff() {
    for (int i = 0; i < NUM_DEBUFF; i++) {
        if (stage.debuffList[i].id == 0) continue;

        switch (stage.debuffList[i].id - 1) {
            case BLEEDING:
                apply_bleeding();
                break;
            case WEAK:
                apply_weak();
                break;
            case CONFUSION:
                apply_confusion();
                break;
            case DARKNESS:
                apply_darkness();
                break;
            case CHILLED:
                apply_chilled();
                break;
            default:
                break;
        }
    }
}


void apply_bleeding() {
    stat.enemy_delta_bullet = 1;
}

void apply_weak() {
    stat.player_delta_bullet = -0.5;
}

void apply_confusion() {
    stat.player_delta_dx = getRandomNumber(-PLAYER_SPEED, PLAYER_SPEED);
    stat.player_delta_dy = getRandomNumber(-PLAYER_SPEED, PLAYER_SPEED);;
}

void apply_darkness() {
    stat.alpha = 80;
}

void apply_chilled() {
    stat.player_delta = 2;
}

void reset_bleeding() {
    stat.enemy_delta_bullet = 0;
}

void reset_weak() {
    stat.player_delta_bullet = 0;
};

void reset_confusion() {
    stat.player_delta_dx = 0;
    stat.player_delta_dy = 0;
};

void reset_darkness() {
    stat.alpha = 255;
}

void reset_chilled() {
    stat.player_delta = 0;
}

void reset_debuff() {
    memset(stage.debuffList, 0, sizeof(Debuff) * NUM_DEBUFF);
    reset_bleeding();
    reset_weak();
    reset_confusion();
    reset_darkness();
    if (stat.player_delta > 0) reset_chilled();
}
