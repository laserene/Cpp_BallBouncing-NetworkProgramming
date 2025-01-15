#include "cstdio"
#include "cstring"
#include "sys/socket.h"

#include "utils.h"
#include "auth.h"

#define ACCOUNT_FILE "_server/accounts.txt"

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

    pthread_mutex_lock(&clientMutex);
    char stored_username[50] = {}, stored_password[50] = {};
    while (fscanf(file, "%s %s %d", stored_username, stored_password, client_id) != EOF) {
        if (strcmp(username, stored_username) == 0) {
            // password check
            if (strcmp(password, stored_password) != 0) {
                fclose(file);
                pthread_mutex_unlock(&clientMutex);
                return 1;
            }

            fclose(file);
            pthread_mutex_unlock(&clientMutex);
            return 2;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&clientMutex);
    return 0;
}

void register_user(const char *username, const char *password) {
    FILE *file = fopen(ACCOUNT_FILE, "a");
    if (!file) {
        perror("Failed to open account file");
        return;
    }

    const int client_id = getRandomNumber();
    pthread_mutex_lock(&clientMutex);
    // FORMAT: username password client_id
    fprintf(file, "%s %s %d\n", username, password, client_id);
    pthread_mutex_unlock(&clientMutex);

    fclose(file);
}