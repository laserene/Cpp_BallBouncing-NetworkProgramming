#ifndef STAGE_H
#define STAGE_H

void initStage();
static void initPlayer();

static void logic();
static void doPlayer();
static void fireBullet();
static void doFighters();
static void doBullets();
static void spawnEnemies();

static void draw();
static void drawPlayer();
static void drawBullets();
static void drawFighters();

#endif //STAGE_H
