# Compiler and Flags
CC = g++
CFLAGS = -Wall -pthread

# Targets
all: server client

# Server Target
server: server.o auth.o server_helpers.o
	$(CC) $(CFLAGS) -o server server.o auth.o server_helpers.o

# Client Target
client: client.o
	$(CC) $(CFLAGS) -o client client.o

# Object Files
server.o: _server/server.cpp _server/auth.h
	$(CC) $(CFLAGS) -c _server/server.cpp -o server.o

client.o: _client/client.cpp
	$(CC) $(CFLAGS) -c _client/client.cpp -o client.o

server_helpers.o: _server/server_helpers.cpp _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/server_helpers.cpp -o server_helpers.o

auth.o: _server/auth.cpp _server/auth.h _server/server_helpers.h
	$(CC) $(CFLAGS) -c _server/auth.cpp -o auth.o

# Clean Rule
clean:
	rm -f *.o server client
