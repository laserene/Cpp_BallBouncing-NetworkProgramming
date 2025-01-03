#include "SDL2/SDL_ttf.h"
#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "structs.h"
#include "stage.h"
#include "text.h"

enum GameState { MENU, PLAYING, EXIT };

GameState gameState = MENU;
int selectedOption = 0;

char *menuOptions[] = {"1. Play Game", "2. Exit"};
constexpr int menuOptionCount = 2;

void renderMenu();

void handleMenuInput(SDL_Event *event);

static void capFrameRate(long *then, float *remainder);

int main(int argc, char *argv[]) {
    memset(&app, 0, sizeof(App));

    initSDL();
    initStage();
    initFonts();

    long then = SDL_GetTicks();
    float remainder = 0;

    while (true) {
        prepareScene();

        SDL_Event event;
        if (gameState == MENU) {
            while (SDL_PollEvent(&event)) {
                handleMenuInput(&event);
            }
        }

        if (gameState == PLAYING) {
            doInput();
            app.delegate.logic();
            app.delegate.draw();
        }

        presentScene();

        capFrameRate(&then, &remainder);

        if (gameState == EXIT) {
            break;
        }
    }

    return 0;
}

void handleMenuInput(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_KP_1:
                gameState = PLAYING;
                break;
            case SDLK_KP_2:
                gameState = EXIT;
                break;
            default:
                break;
        }
    }
}

static void capFrameRate(long *then, float *remainder) {
    long wait = 16 + *remainder;
    *remainder -= static_cast<int>(*remainder);

    const long frameTime = SDL_GetTicks() - *then;

    wait -= frameTime;

    if (wait < 1) {
        wait = 1;
    }

    SDL_Delay(wait);

    *remainder += 0.667;

    *then = SDL_GetTicks();
}
