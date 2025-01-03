#ifndef COMMON_H
#define COMMON_H

#include "structs.h"

inline App app;
inline GameState gameState = MENU;
inline ModeState modeState = RANDOM;

// Map
inline SDL_Texture *spaceTexture;
inline SDL_Texture *hallowTexture;
inline SDL_Texture *snowTexture;

// Enemy
inline SDL_Texture *mushroomTexture;
inline SDL_Texture *umbrellaTexture;
inline SDL_Texture *snowmanTexture;

// Enemy Bullet
inline SDL_Texture *bombTexture;
inline SDL_Texture *eggTexture;
inline SDL_Texture *candycaneTexture;

#endif //COMMON_H
