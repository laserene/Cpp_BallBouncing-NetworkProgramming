#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "arpa/inet.h"

#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "client.h"

#define BUFFER_SIZE 1024

App app;

void handle_communication(const int sock) {
    // Socket handler
    char buffer[BUFFER_SIZE] = {};
    fd_set read_fds; // File descriptors, each represent a socket

    // App handler
    memset(&app, 0, sizeof(App));
    initSDL();
    atexit(cleanup);

    while (true) {
        prepareScene();
        doInput();

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sock + 1, &read_fds, nullptr, nullptr, nullptr) < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            if (fgets(buffer, BUFFER_SIZE, stdin) == nullptr) {
                break;
            }
            send(sock, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            if (const int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0); bytes_received <= 0) {
                printf("Disconnected from server\n");
                break;
            }
            printf("%s", buffer);
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

    printf("Connected to the server.\n");
    handle_communication(sock);
    close(sock);
    return 0;
}