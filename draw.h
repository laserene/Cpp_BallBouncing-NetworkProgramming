#ifndef DRAW_H
#define DRAW_H

void prepareScene();
void presentScene();
SDL_Texture *loadTexture(char *filename);
void blit(SDL_Texture *texture, int x, int y);
void blitRect(SDL_Texture *texture, const SDL_Rect *src, int x, int y);

#endif //DRAW_H
