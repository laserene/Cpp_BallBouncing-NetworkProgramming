#include "handle_client_message.h"
#include "cstdio"
#include "cstring"

char *handle_move_message(char *buffer) {
    char direction[12];
    sscanf(buffer, "MOVE %s", direction);

    if (strcmp(direction, "UP") == 0) {
        snprintf(buffer, sizeof(buffer), "UPDATE %d %d %d %d", 4, 0, 0, 0);
        return buffer;
    }

    if (strcmp(direction, "DOWN") == 0) {
        snprintf(buffer, sizeof(buffer), "UPDATE %d %d %d %d", 0, -4, 0, 0);
        return buffer;
    }

    if (strcmp(direction, "RIGHT") == 0) {
        snprintf(buffer, sizeof(buffer), "UPDATE %d %d %d %d", 0, 0, 4, 0);
        return buffer;
    }

    if (strcmp(direction, "LEFT") == 0) {
        snprintf(buffer, sizeof(buffer), "UPDATE %d %d %d %d", 0, 0, 0, -4);
        return buffer;
    }

    snprintf(buffer, sizeof(buffer), "UPDATE %d %d %d %d", 0, 0, 0, 0);
    return buffer;
}
