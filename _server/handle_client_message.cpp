#include "cstdio"

#include "handle_client_message.h"
#include "messages.h"
#include "defs.h"

char *handle_move_message(char *buffer) {
    // MOVE message format: up, down, left, right
    int u, d, l, r;
    sscanf(buffer, CLIENT_MOVE, &u, &d, &l, &r);

    if (u != 0) {
        snprintf(buffer, BUFFER_SIZE, REPLY_CLIENT_UPDATE, 4, 0, 0, 0);
        return buffer;
    }

    if (d != 0) {
        snprintf(buffer, BUFFER_SIZE, REPLY_CLIENT_UPDATE, 0, -4, 0, 0);
        return buffer;
    }

    if (l != 0) {
        snprintf(buffer, BUFFER_SIZE, REPLY_CLIENT_UPDATE, 0, 0, -4, 0);
        return buffer;
    }

    if (r != 0) {
        snprintf(buffer, BUFFER_SIZE, REPLY_CLIENT_UPDATE, 0, 0, 0, 4);
        return buffer;
    }

    snprintf(buffer, BUFFER_SIZE, REPLY_CLIENT_UPDATE, 0, 0, 0, 0);
    return buffer;
}
