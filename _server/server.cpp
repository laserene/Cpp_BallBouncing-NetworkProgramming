#include "iostream"
#include "cstdlib"
#include "cstring"
#include "unistd.h"
#include "pthread.h"
#include "arpa/inet.h"

#include "defs.h"
#include "auth.h"
#include "messages.h"
#include "handle_client_message.h"

void *handleClient(void *clientSocketPtr) {
    const int clientSocket = *static_cast<int *>(clientSocketPtr);
    char buffer[BUFFER_SIZE] = {};
    char username[50] = {}, password[50] = {};
    int client_id;

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);

        // Remove disconnected client
        if (const ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); bytesReceived <= 0) {
            printf("Client disconnected.\n");
            close(clientSocket);

            pthread_mutex_lock(&clientMutex);
            for (int & client : clients) {
                if (client == clientSocket) {
                    client = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&clientMutex);
            break;
        }

        std::cout << "CMD: " << buffer << std::endl;

        // Receive REGISTER, LOGIN messages
        if (strncmp(buffer, "REGISTER", 8) == 0) {
            // sscanf the string following REGISTER
            sscanf(buffer + 9, "%s %s", username, password);
            if (strlen(username) == 0 || strlen(password) == 0) {
                send(clientSocket, REPLY_USER_CREDENTIALS_NOT_BLANK, strlen(REPLY_USER_CREDENTIALS_NOT_BLANK), 0);
                continue;
            }

            if (check_existing_user(username, password, &client_id) != 0) {
                send(clientSocket, REPLY_USER_EXISTED, strlen(REPLY_USER_EXISTED), 0);
                continue;
            }

            register_user(username, password);
            send(clientSocket, REPLY_REGISTRATION_SUCCEED, strlen(REPLY_REGISTRATION_SUCCEED), 0);
        } else if (strncmp(buffer, "LOGIN", 5) == 0) {
            sscanf(buffer + 6, "%s %s", username, password);

            // user not found
            if (check_existing_user(username, password, &client_id) == 0) {
                send(clientSocket, REPLY_USER_NOT_EXISTED, strlen(REPLY_USER_NOT_EXISTED), 0);
                continue;
            }

            // wrong password
            if (check_existing_user(username, password, &client_id) == 1) {
                send(clientSocket, REPLY_USER_CREDENTIALS_NOT_MATCHED, strlen(REPLY_USER_CREDENTIALS_NOT_MATCHED), 0);
                continue;
            }

            send(clientSocket, REPLY_LOGIN_SUCCEED, strlen(REPLY_LOGIN_SUCCEED), 0);
            break;
        }

        // Handle MOVE message
        if (strncmp(buffer, "MOVE", 4) == 0) {
            strcpy(buffer, handle_move_message(buffer));
            send(clientSocket, buffer, strlen(buffer), 0);
        }

        // Handle LEADERBOARD message
        // if (strncmp(buffer, "LEADERBOARD", 11) == 0) {
        //     sscanf(buffer + 12, "%s", buffer);
        //     // Handle get
        //     if (strncmp(buffer, "GET", 3) == 0) {
        //
        //     }
        //
        //     // Handle update
        // }
    }



    return nullptr;
}

void acceptClients(const int serverSocket) {
    while (true) {
        // Accept client
        sockaddr_in clientAddr = {};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientLen);
        if (clientSocket < 0) {
            perror("Error accepting client connection");
            continue;
        }

        // Add client to clients array
        pthread_mutex_lock(&clientMutex);
        int added = 0;
        for (int & client : clients) {
            if (client == 0) {
                client = clientSocket;
                added = 1;
                break;
            }
        }
        pthread_mutex_unlock(&clientMutex);

        if (added) {
            printf("Client connected.\n");
            pthread_t clientThread;
            pthread_create(&clientThread, nullptr, handleClient, &clientSocket);
            // Left clientThread self-running
            pthread_detach(clientThread);
        } else {
            printf("Maximum clients connected. Connection refused.\n");
            close(clientSocket);
        }
    }
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return -1;
    }

    // Init server socket
    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        return -1;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    // Binding
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return -1;
    }

    // Listening
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(serverSocket);
        return -1;
    }
    printf("Server listening on port %d...\n", atoi(argv[1]));

    acceptClients(serverSocket);
    close(serverSocket);

    return 0;
}
