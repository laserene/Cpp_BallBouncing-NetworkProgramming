#include "SDL2/SDL_ttf.h"
#include "common.h"
#include "defs.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "structs.h"

int main(int argc, char *argv[]) {
    memset(&app, 0, sizeof(App));

    initSDL();

    while (true) {
        prepareScene();
        doInput();
        presentScene();

        SDL_Delay(16);
    }

    return 0;
}
