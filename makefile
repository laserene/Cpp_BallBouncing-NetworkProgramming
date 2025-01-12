# Compiler and Flags
CC = g++
CFLAGS = -Wall -pthread
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

# Targets
all: server client

# Server Target
server: server.o auth.o server_helpers.o handle_client_message.o
	$(CC) $(CFLAGS) -o server server.o auth.o server_helpers.o handle_client_message.o

# Client Target
client: client.o input.o init.o draw.o stage.o util.o
	$(CC) $(CFLAGS) -o client client.o input.o init.o draw.o stage.o util.o $(LDFLAGS)

# Object Files
server.o: _server/server.cpp _server/auth.h _server/defs.h
	$(CC) $(CFLAGS) -c _server/server.cpp -o server.o

client.o: _client/client.cpp
	$(CC) $(CFLAGS) -c _client/client.cpp -o client.o $(LDFLAGS)

handle_client_message.o: _server/handle_client_message.cpp _server/handle_client_message.h
	$(CC) $(CFLAGS) -c _server/handle_client_message.cpp -o handle_client_message.o

server_helpers.o: _server/server_helpers.cpp _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/server_helpers.cpp -o server_helpers.o

auth.o: _server/auth.cpp _server/auth.h _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/auth.cpp -o auth.o

input.o: _client/input.cpp _client/input.h _client/common.h
	$(CC) $(CFLAGS) -c _client/input.cpp -o input.o $(LDFLAGS)

init.o: _client/init.cpp _client/init.h _client/common.h _client/defs.h
	$(CC) $(CFLAGS) -c _client/init.cpp -o init.o $(LDFLAGS)

draw.o: _client/draw.cpp _client/draw.h _client/common.h
	$(CC) $(CFLAGS) -c _client/draw.cpp -o draw.o $(LDFLAGS)

stage.o: _client/stage.cpp _client/stage.h _client/common.h
	$(CC) $(CFLAGS) -c _client/stage.cpp -o stage.o

util.o: _client/util.cpp _client/util.h
	$(CC) $(CFLAGS) -c _client/util.cpp -o util.o

# Clean Rule
clean:
	rm -f *.o server client
