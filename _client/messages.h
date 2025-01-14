#ifndef MESSAGES_H
#define MESSAGES_H

// SEND MESSAGES
inline auto SEND_LOGIN = "LOGIN %s %s\n";
inline auto SEND_REGISTER = "REGISTER %s %s\n";
inline auto SEND_MOVE = "MOVE %d %d %d %d\n";

// SERVER MESSAGES
inline auto SERVER_UPDATE = "UPDATE %d %d %d %d\n";

#endif //MESSAGES_H
