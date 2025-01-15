#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "pthread.h"
#include "iostream"
#include "arpa/inet.h"
#include "sys/socket.h"

#include "auth.h"
#include "messages.h"
#include "handle_client_message.h"
#include "defs.h"

void *handle_client(void *arg) {
    const int client_sock = *static_cast<int *>(arg);
    char buffer[BUFFER_SIZE] = {};
    char username[50] = {}, password[50] = {};
    int client_id;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        if (const ssize_t byteRecv = recv(client_sock, buffer, BUFFER_SIZE, 0); byteRecv <= 0) {
            // Close client socket
            pthread_mutex_lock(&mutex);
            for (int &client_socket: client_sockets) {
                if (client_socket == client_sock) {
                    client_socket = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            close(client_sock);
            printf("Client disconnected\n");
            return nullptr;
        }

        // Receive REGISTER, LOGIN messages
        if (strncmp(buffer, "REGISTER", 8) == 0) {
            // sscanf the string following REGISTER
            sscanf(buffer + 9, "%s %s", username, password);
            if (strlen(username) == 0 || strlen(password) == 0) {
                send(client_sock, REPLY_USER_CREDENTIALS_NOT_BLANK, strlen(REPLY_USER_CREDENTIALS_NOT_BLANK), 0);
                continue;
            }

            if (check_existing_user(username, password, &client_id) != 0) {
                send(client_sock, REPLY_USER_EXISTED, strlen(REPLY_USER_EXISTED), 0);
                continue;
            }

            register_user(username, password);
            send(client_sock, REPLY_REGISTRATION_SUCCEED, strlen(REPLY_REGISTRATION_SUCCEED), 0);
        } else if (strncmp(buffer, "LOGIN", 5) == 0) {
            sscanf(buffer + 6, "%s %s", username, password);

            // user not found
            if (check_existing_user(username, password, &client_id) == 0) {
                send(client_sock, REPLY_USER_NOT_EXISTED, strlen(REPLY_USER_NOT_EXISTED), 0);
                continue;
            }

            // wrong password
            if (check_existing_user(username, password, &client_id) == 1) {
                send(client_sock, REPLY_USER_CREDENTIALS_NOT_MATCHED, strlen(REPLY_USER_CREDENTIALS_NOT_MATCHED), 0);
                continue;
            }

            send(client_sock, REPLY_LOGIN_SUCCEED, strlen(REPLY_LOGIN_SUCCEED), 0);
            break;
        }
    }

    while (true) {
        // Broadcast message on any user pushing to server
        memset(buffer, 0, BUFFER_SIZE);
        if (const ssize_t byteRecv = recv(client_sock, buffer, BUFFER_SIZE, 0); byteRecv <= 0) {
            // Close client socket
            pthread_mutex_lock(&mutex);
            for (int &client_socket: client_sockets) {
                if (client_socket == client_sock) {
                    client_socket = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            close(client_sock);
            printf("Client disconnected\n");
            return nullptr;
        }

        std::cout << buffer << std::endl;

        // Handle move message
        if (strncmp(buffer, "MOVE", 4) == 0) {
            strcpy(buffer, handle_move_message(buffer));
            send(client_sock, buffer, strlen(buffer), 0);
        }

        // Handle leaderboard message
        if (strncmp(buffer, "LEADERBOARD", 11) == 0) {
            sscanf(buffer + 12, "%s", buffer);

            // Handle get
            if (strncmp(buffer, "GET", 3) == 0) {

            }

            // Handle update
        }
    }

    return nullptr;
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return -1;
    }

    // Init server sock
    const int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Server socket creation failed");
        return -1;
    }

    // Set up server address
    sockaddr_in server_addr = {};
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

    int client_sock;
    sockaddr_in client_addr = {};
    socklen_t client_len = sizeof(client_addr);
    // Loop
    while (true) {
        // New client sock
        client_sock = accept(server_sock, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }

        // Add new client to client list
        pthread_mutex_lock(&mutex);
        int added = 0;
        for (int &client_socket: client_sockets) {
            if (client_socket == 0) {
                client_socket = client_sock;
                added = 1;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        // Server full check
        if (!added) {
            send(client_sock, REPLY_SERVER_FULL, strlen(REPLY_SERVER_FULL), 0);
            close(client_sock);
            continue;
        }

        // Create a thread for new client
        printf("New client connected\n");
        pthread_create(&threads[thread_index++], nullptr, handle_client, &client_sock);
    }

    close(server_sock);
    return 0;
}
