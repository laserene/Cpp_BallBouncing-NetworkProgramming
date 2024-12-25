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

#endif //STAGE_H
