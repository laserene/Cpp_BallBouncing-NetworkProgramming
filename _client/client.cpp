#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "iostream"
#include "fcntl.h"
#include "unistd.h"
#include "arpa/inet.h"

#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "messages.h"
#include "client.h"

#define BUFFER_SIZE 1024

App app;
Entity player;
Entity bullet;

void handle_server_message(const char *buffer) {
    if (strncmp(buffer, "UPDATE", 6) == 0) {
        int u, d, l, r;
        sscanf(buffer, SERVER_UPDATE, &u, &d, &l, &r);
        player.y += u;
        player.y += d;
        player.x += l;
        player.x += r;
    } else {
        std::cout << buffer << std::endl;
    }
}

void handle_communication(const int sock) {
    // Socket handler
    char buffer[BUFFER_SIZE] = {};
    fd_set read_fds = {};

    // Timeout for select
    timeval timeout = {};
    timeout.tv_sec = 0; // 0 seconds
    timeout.tv_usec = 16000;

    // Reset game components
    memset(&app, 0, sizeof(App));
    memset(&player, 0, sizeof(Entity));
    memset(&bullet, 0, sizeof(Entity));

    // Init SDL
    initSDL();

    // Initial components setting
    player.x = 100;
    player.y = 100;
    player.texture = loadTexture(PLAYER_TEXTURE);
    bullet.texture = loadTexture(BULLET_TEXTURE);

    atexit(cleanup);

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sock + 1, &read_fds, nullptr, nullptr, &timeout) < 0) {
            perror("Select error");
            break;
        }

        prepareScene();
        doInput();

        // Handle character movement
        if (app.up) {
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), SEND_MOVE, 1, 0, 0, 0);
            send(sock, buffer, BUFFER_SIZE, 0);
        }

        if (app.down) {
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 1, 0, 0);
            send(sock, buffer, BUFFER_SIZE, 0);
        }

        if (app.left) {
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 0, 1, 0);
            send(sock, buffer, BUFFER_SIZE, 0);
        }

        if (app.right) {
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, sizeof(buffer), SEND_MOVE, 0, 0, 0, 1);
            send(sock, buffer, strlen(buffer), 0);
        }

        // Fire another bullet once bullet is out of screen
        if (app.fire && bullet.health == 0) {
            bullet.x = player.x;
            bullet.y = player.y;
            bullet.dx = 16;
            bullet.dy = 0;
            bullet.health = 1;
        }

        // Update bullet movement
        bullet.x += bullet.dx;
        bullet.y += bullet.dy;

        if (bullet.x > SCREEN_WIDTH) {
            bullet.health = 0;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            if (fgets(buffer, BUFFER_SIZE, stdin) == nullptr) {
                break;
            }
            send(sock, buffer, BUFFER_SIZE, 0);
        }

        if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            if (const int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0); bytes_received <= 0) {
                printf("Disconnected from server\n");
                break;
            }
            handle_server_message(buffer);
        }

        blit(player.texture, player.x, player.y);

        if (bullet.health > 0) {
            blit(bullet.texture, bullet.x, bullet.y);
        }

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

    // Non-blocking socket
    fcntl(sock, F_SETFL, O_NONBLOCK);

    printf("Connected to the server.\n");
    handle_communication(sock);
    close(sock);
    return 0;
}
