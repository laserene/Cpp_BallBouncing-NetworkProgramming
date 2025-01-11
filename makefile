# Compiler and Flags
CC = g++
CFLAGS = -Wall -pthread
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

# Targets
all: server client

# Server Target
server: server.o auth.o server_helpers.o
	$(CC) $(CFLAGS) -o server server.o auth.o server_helpers.o

# Client Target
client: client.o init.o
	$(CC) $(CFLAGS) -o client client.o init.o $(LDFLAGS)

# Object Files
server.o: _server/server.cpp _server/auth.h
	$(CC) $(CFLAGS) -c _server/server.cpp -o server.o

client.o: _client/client.cpp _client/init.h _client/common.h _client/defs.h _client/structs.h
	$(CC) $(CFLAGS) -c _client/client.cpp -o client.o

init.o: _client/init.cpp _client/init.h
	$(CC) $(CFLAGS) -c _client/init.cpp -o init.o

server_helpers.o: _server/server_helpers.cpp _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/server_helpers.cpp -o server_helpers.o

auth.o: _server/auth.cpp _server/auth.h _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/auth.cpp -o auth.o

# Clean Rule
clean:
	rm -f *.o server client
