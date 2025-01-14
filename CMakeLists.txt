cmake_minimum_required(VERSION 3.29)
project(auth)

set(CMAKE_CXX_STANDARD 20)

add_executable(auth _client/client.cpp
        _server/server.cpp
        _server/messages.h
        _server/server_helpers.cpp
        _server/server_helpers.h
        _server/auth.cpp
        _server/auth.h
        _client/defs.h
        _client/structs.h
        _client/init.cpp
        _client/init.h
        _client/common.h
        _client/init.cpp
        _client/draw.cpp
        _client/draw.h
        _client/input.cpp
        _client/input.h
        _client/client.h
        _client/paths.h
        _server/handle_client_message.cpp
        _server/handle_client_message.h
        _client/messages.h
        _server/defs.h
        _client/stage.cpp
        _client/stage.h
        _client/util.cpp
        _client/util.h
        _client/stage.cpp
        _client/text.cpp
        _client/text.h
        _client/helper.cpp
        _client/helper.h
        _client/buff.h
        _client/debuff.h
        _client/welcome.h
        _client/auth.h
)
