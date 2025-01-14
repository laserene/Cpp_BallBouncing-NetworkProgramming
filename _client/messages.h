#ifndef MESSAGES_H
#define MESSAGES_H

// SEND MESSAGES
inline auto SEND_LOGIN = "LOGIN %s %s\n";
inline auto SEND_REGISTER = "REGISTER %s %s\n";
inline auto SEND_MOVE = "MOVE %d %d %d %d\n";

// SERVER MESSAGES
inline auto SERVER_UPDATE = "UPDATE %d %d %d %d\n";
inline auto SERVER_FULL = "SERVER FULL\n";

inline auto AUTH_USER_EXISTED = "AUTH USER_EXISTED\n";
inline auto AUTH_USER_NOT_EXISTED = "AUTH USER_NOT_EXISTED\n";
inline auto AUTH_USER_CREDENTIALS_NOT_MATCHED = "AUTH WRONG_CREDENTIALS\n";

inline auto AUTH_REGISTRATION_SUCCEED = "AUTH REGISTRATION_SUCCESS\n";
inline auto AUTH_LOGIN_SUCCEED = "AUTH LOGIN_SUCCESS\n";

#endif //MESSAGES_H
