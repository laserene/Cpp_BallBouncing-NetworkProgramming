#include "SDL2/SDL_ttf.h"
#include "common.h"
#include "defs.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "structs.h"

int main(int argc, char *argv[]) {
    memset(&app, 0, sizeof(App));
    memset(&player, 0, sizeof(Entity));
    memset(&ball, 0, sizeof(Entity));

    initSDL();

    char *messi = "../gfx/messi.png";

    // Init player at middle bottom
    player.x = SCREEN_WIDTH / 2;
    player.y = SCREEN_HEIGHT - 200;
    player.texture = loadTexture(messi);

    ball.texture = loadTexture("../gfx/ball.png");

    while (true) {
        prepareScene();

        doInput();

        player.x += player.dx;
        player.y += player.dy;

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

        if (app.fire && ball.health == 0) {
            ball.x = player.x;
            ball.y = player.y;
            ball.dx = 16;
            ball.dy = 0;
            ball.health = 1;
        }

        ball.x += ball.dx;
        ball.y += ball.dy;

        if (ball.x > SCREEN_WIDTH) {
            ball.health = 0;
        }

        blit(player.texture, player.x, player.y);

        if (ball.health > 0) {
            blit(ball.texture, ball.x, ball.y);
        }

        presentScene();

        SDL_Delay(16);
    }


    return 0;
}
