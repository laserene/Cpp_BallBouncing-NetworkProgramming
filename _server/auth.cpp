#include "cstdio"
#include "cstring"
#include "sys/socket.h"

#include "server_helpers.h"
#include "auth.h"

#define ACCOUNT_FILE "_server/accounts.txt"

void broadcast_message(const char *message, const int sender_sock) {
    pthread_mutex_lock(&mutex);
    for (const int client_socket: client_sockets) {
        if (client_socket != 0 && client_socket != sender_sock) {
            send(client_socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

int check_existing_user(const char *username, const char *password, int *client_id) {
    /*
     * Return: 0 if user not existed
     *         1 if wrong password
     *         2 if a user matched
     */
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (!file) {
        perror("Failed to open account file");
        return 0;
    }

    pthread_mutex_lock(&mutex);
    char stored_username[50] = {}, stored_password[50] = {};
    while (fscanf(file, "%s %s %d", stored_username, stored_password, client_id) != EOF) {
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