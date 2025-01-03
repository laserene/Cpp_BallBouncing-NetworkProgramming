#ifndef STAGE_H
#define STAGE_H

#include "structs.h"

void initStage();
static void initPlayer();
static void resetStage();

static void logic();
static void doPlayer();
static void fireBullet();
static void doFighters();
static void doBullets();
static void spawnEnemies();
static int bulletHitFighter(Entity *b);
static void doEnemies();
static void fireAlienBullet(Entity *e);
static void clipPlayer();
static void initStarfield();
static void doBackground();
static void doStarfield();
static void doExplosions();
static void doDebris();
static void addExplosions(int x, int y, int num);
static void addDebris(Entity *e);
static void doPointsPods();
static void addPointsPod(int x, int y);
static void consumeDebuff(int id);

static void draw();
static void drawBackground();
static void drawStarfield();
static void drawPointsPods();
static void drawPlayer();
static void drawBullets();
static void drawFighters();
static void drawDebris();
static void drawExplosions();
static void drawHud();
static void drawHeart(int x, int y);
static void drawDebuff(int x, int y);
static void drawBuff(int x, int y);

// Buff
void doBuff();
void apply_add_bullet();
void apply_freeze();
void apply_speedup();
void apply_luck();
void apply_refresh();

void reset_add_bullet();
void reset_freeze();
void reset_speedup();
void reset_luck();
void reset_refresh();

// Buff
void doBuff();
void apply_add_bullet();
void apply_freeze();
void apply_speedup();
void apply_luck();
void apply_refresh();

void reset_add_bullet();
void reset_freeze();
void reset_speedup();
void reset_luck();
void reset_refresh();
void reset_debuff();

// Debuff
void doDebuff();
void apply_bleeding();
void apply_weak();
void apply_confusion();
void apply_darkness();
void apply_chilled();

void reset_bleeding();
void reset_weak();
void reset_confusion();
void reset_darkness();
void reset_chilled();

#endif //STAGE_H
