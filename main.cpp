#include "SDL2/SDL_ttf.h"
#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "structs.h"

int main(int argc, char *argv[]) {
    memset(&app, 0, sizeof(App));
    memset(&player, 0, sizeof(Entity));

    initSDL();

    char *messi = "../gfx/messi.png";

    player.x = 100;
    player.y = 100;
    player.texture = loadTexture(messi);

    while (true) {
        prepareScene();

        doInput();

        if (app.up) {
            player.y -= 4;
        }

        if (app.down) {
            player.y += 4;
        }

        if (app.left) {
            player.x -= 4;
        }

        if (app.right) {
            player.x += 4;
        }

        blit(player.texture, player.x, player.y);
        presentScene();

        SDL_Delay(16);
    }


    return 0;
}
