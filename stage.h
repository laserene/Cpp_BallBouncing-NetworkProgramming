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

static void draw();
static void drawPlayer();
static void drawBullets();
static void drawFighters();

#endif //STAGE_H
