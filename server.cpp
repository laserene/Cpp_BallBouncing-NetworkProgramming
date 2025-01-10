#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "pthread.h"
#include "arpa/inet.h"
#include "fcntl.h"
#include "sys/socket.h"

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define ACCOUNT_FILE "accounts.txt"

// Init mutex and array to store client sockets
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS] = {};

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return -1;
    }

    int client_sock;
    sockaddr_in server_addr = {}, client_addr = {};
    socklen_t client_len = sizeof(client_addr);

    // Init server sock
    const int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Server socket creation failed");
        return -1;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return -1;
    }

    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        return -1;
    }

    printf("Server is running on port %s\n", argv[1]);

    pthread_t threads[MAX_CLIENTS];
    int thread_index = 0;

    while (true) {
        client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&mutex);
        int added = 0;
        for (int &client_socket : client_sockets) {
            if (client_socket == 0) {
                client_socket = client_sock;
                added = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        if (!added) {
            send(client_sock, "Server full", 12, 0);
            close(client_sock);
            continue;
        }

        printf("New client connected\n");
        pthread_create(&threads[thread_index++], NULL, handle_client, &client_sock);
    }

    close(server_sock);
    return 0;
}
