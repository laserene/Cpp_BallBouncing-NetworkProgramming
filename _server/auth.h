#ifndef AUTH_H
#define AUTH_H

#include "pthread.h"

#define MAX_CLIENTS 5

// Init mutex and array to store client sockets
inline int clients[MAX_CLIENTS] = {};
inline pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER;

void register_user(const char *username, const char *password);
int check_existing_user(const char *username, const char *password, int *client_id);

#endif //AUTH_H
