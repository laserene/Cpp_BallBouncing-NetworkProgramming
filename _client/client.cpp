#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "iostream"
#include "unistd.h"
#include "arpa/inet.h"

#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "client.h"

#define BUFFER_SIZE 1024

App app;
Entity player;

void handle_communication(const int sock) {
    // Socket handler
    char buffer[BUFFER_SIZE] = {};
    fd_set read_fds = {};

    timeval timeout = {};
    timeout.tv_sec = 0; // 0 seconds
    timeout.tv_usec = 16000;

    // App handler
    memset(&app, 0, sizeof(App));
    memset(&player, 0, sizeof(Entity));

    initSDL();

    player.x = 100;
    player.y = 100;
    player.texture = loadTexture(PLAYER_TEXTURE);

    atexit(cleanup);

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        select(sock + 1, &read_fds, nullptr, nullptr, &timeout);

        prepareScene();
        doInput();

        if (app.up) {
            player.y -= 4;
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), "MOVE UP");
            send(sock, buffer, strlen(buffer), 0);
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

        memset(buffer, 0, BUFFER_SIZE);
        if (const size_t bytes_received = recv(sock, buffer, BUFFER_SIZE, 0); bytes_received <= 0) {
            printf("Disconnected from server\n");
            break;
        }

        blit(player.texture, player.x, player.y);

        presentScene();
        SDL_Delay(16);
    }
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        return -1;
    }

    // Set up client socket
    const int sock = socket(AF_INET, SOCK_STREAM, 0);

    // Set up remote server address
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
        perror("Connection failed!");
        return 1;
    }

    printf("Connected to the server.\n");
    handle_communication(sock);
    close(sock);
    return 0;
}
