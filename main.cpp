#include "SDL2/SDL_ttf.h"
#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "structs.h"
#include "stage.h"
#include "text.h"

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

        doInput();

        app.delegate.logic();

        app.delegate.draw();

        presentScene();

        capFrameRate(&then, &remainder);
    }

    return 0;
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
