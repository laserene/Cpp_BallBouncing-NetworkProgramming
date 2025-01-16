#ifndef STAGE_H
#define STAGE_H

#include "structs.h"

inline Entity *player;

void initStage();
void handle_server_play_message(const char *buffer);
void doBackground();
void doStarfield();
void drawBackground();
void drawStarfield();
void resetStage();

#endif //STAGE_H
