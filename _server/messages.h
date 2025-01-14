#ifndef MESSAGES_H
#define MESSAGES_H

// CLIENT MESSAGES
inline auto CLIENT_MOVE = "MOVE %d %d %d %d\n";


// REPLY MESSAGES
inline auto REPLY_SERVER_FULL = "SERVER FULL\n";

inline auto REPLY_USER_EXISTED = "AUTH USER_EXISTED\n";
inline auto REPLY_USER_NOT_EXISTED = "AUTH USER_NOT_EXISTED\n";
inline auto REPLY_USER_CREDENTIALS_NOT_MATCHED = "AUTH WRONG_CREDENTIALS\n";
inline auto REPLY_USER_CREDENTIALS_NOT_BLANK = "AUTH BLANK_CREDENTIALS\n";

inline auto REPLY_REGISTRATION_SUCCEED = "AUTH REGISTRATION_SUCCESS\n";
inline auto REPLY_LOGIN_SUCCEED = "AUTH LOGIN_SUCCESS\n";

inline auto REPLY_CLIENT_UPDATE = "UPDATE %d %d %d %d\n";

#endif //MESSAGES_H
