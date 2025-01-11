#include "cstdio"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "pthread.h"
#include "arpa/inet.h"
#include "sys/socket.h"

#include "server_helpers.h"
#include "messages.h"

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define ACCOUNT_FILE "accounts.txt"

// Init mutex and array to store client sockets
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS] = {};


void broadcast_message(const char *message, const int sender_sock) {
    pthread_mutex_lock(&mutex);
    for (const int client_socket: client_sockets) {
        if (client_socket != 0 && client_socket != sender_sock) {
            send(client_socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

int check_existing_user(const char *username, const char *password) {
    /*
     * Return: 0 if user not existed
     *         1 if wrong password
     *         2 if a user matched
     */
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (!file) {
        perror("Failed to open account file");
        return false;
    }

    pthread_mutex_lock(&mutex);
    char stored_username[50] = {}, stored_password[50] = {};
    while (fscanf(file, "%s %s", stored_username, stored_password) != EOF) {
        if (strcmp(username, stored_username) == 0) {
            // password check
            if (strcmp(password, stored_password) != 0) {
                fclose(file);
                pthread_mutex_unlock(&mutex);
                return 1;
            }

            fclose(file);
            pthread_mutex_unlock(&mutex);
            return 2;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void register_user(const char *username, const char *password) {
    FILE *file = fopen(ACCOUNT_FILE, "a");
    if (!file) {
        perror("Failed to open account file");
        return;
    }

    const int client_id = getRandomNumber();
    pthread_mutex_lock(&mutex);
    // FORMAT: username password client_id
    fprintf(file, "%s %s %d\n", username, password, client_id);
    pthread_mutex_unlock(&mutex);

    fclose(file);
}

void *handle_client(void *arg) {
    const int client_sock = *static_cast<int *>(arg);
    char buffer[BUFFER_SIZE] = {};
    char username[50] = {}, password[50] = {};
    int client_id;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        // Receive REGISTER, LOGIN messages
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "REGISTER", 8) == 0) {
            // sscanf the string following REGISTER
            sscanf(buffer + 9, "%s %s", username, password);
            if (check_existing_user(username, password) != 0) {
                send(client_sock, REPLY_USER_EXISTED, strlen(REPLY_USER_EXISTED), 0);
                continue;
            }

            register_user(username, password);
            send(client_sock, REPLY_REGISTRATION_SUCCEED, strlen(REPLY_REGISTRATION_SUCCEED), 0);
        } else if (strncmp(buffer, "LOGIN", 5) == 0) {
            sscanf(buffer + 6, "%s %s", username, password);

            // user not found
            if (check_existing_user(username, password) == 0) {
                send(client_sock, REPLY_USER_NOT_EXISTED, strlen(REPLY_USER_NOT_EXISTED), 0);
                continue;
            }

            // wrong password
            if (check_existing_user(username, password) == 1) {
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
        recv(client_sock, buffer, BUFFER_SIZE, 0);
        printf("%s: %s", username, buffer);
        broadcast_message(buffer, client_sock);
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