#ifndef MESSAGES_H
#define MESSAGES_H

// CLIENT MESSAGES
inline auto CLIENT_MOVE = "MOVE %d %d %d %d\n";


// REPLY MESSAGES
inline auto REPLY_SERVER_FULL = "Server full\n";

inline auto REPLY_USER_EXISTED = "User existed\n";
inline auto REPLY_USER_NOT_EXISTED = "User not found\n";
inline auto REPLY_USER_CREDENTIALS_NOT_MATCHED = "Wrong password\n";

inline auto REPLY_REGISTRATION_SUCCEED = "Registration successful\n";
inline auto REPLY_LOGIN_SUCCEED = "Login successful\n";

inline auto REPLY_CLIENT_UPDATE = "UPDATE %d %d %d %d\n";

#endif //MESSAGES_H
