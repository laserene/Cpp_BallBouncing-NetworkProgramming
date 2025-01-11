CC = g++
CFLAGS = -Wall -pthread

# Targets
all: server client

server: server.o server_helpers.o
	$(CC) $(CFLAGS) -o server server.o server_helpers.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

server.o: _server/server.cpp
	$(CC) $(CFLAGS) -c _server/server.cpp

client.o: _client/client.cpp
	$(CC) $(CFLAGS) -c _client/client.cpp

server_helpers.o: _server/server_helpers.cpp
	$(CC) $(CFLAGS) -c _server/server_helpers.cpp

clean:
	rm -f *.o server client