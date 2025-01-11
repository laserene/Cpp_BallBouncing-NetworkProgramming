#ifndef AUTH_H
#define AUTH_H

#include "pthread.h"

#define MAX_CLIENTS 5

// Init mutex and array to store client sockets
inline pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
inline int client_sockets[MAX_CLIENTS] = {};

void broadcast_message(const char *message, int sender_sock);
void register_user(const char *username, const char *password);
int check_existing_user(const char *username, const char *password, int *client_id);

#endif //AUTH_H
