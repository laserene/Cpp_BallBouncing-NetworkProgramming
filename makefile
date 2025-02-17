# Compiler and Flags
CC = g++
CFLAGS = -Wall -pthread
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

# Targets
all: server client

# Server Target
server: server.o auth.o utils.o handle_client_message.o
	$(CC) $(CFLAGS) -o server server.o auth.o utils.o handle_client_message.o

# Client Target
client: client.o input.o init.o draw.o stage.o util.o text.o helper.o
	$(CC) $(CFLAGS) -o client client.o input.o init.o draw.o stage.o util.o text.o helper.o $(LDFLAGS)

# Object Files
server.o: _server/server.cpp _server/auth.h _server/defs.h
	$(CC) $(CFLAGS) -c _server/server.cpp -o server.o

client.o: _client/client.cpp
	$(CC) $(CFLAGS) -c _client/client.cpp -o client.o $(LDFLAGS)

handle_client_message.o: _server/handle_client_message.cpp _server/handle_client_message.h
	$(CC) $(CFLAGS) -c _server/handle_client_message.cpp -o handle_client_message.o

utils.o: _server/utils.cpp _server/utils.h
	$(CC) $(CFLAGS) -c _server/utils.cpp -o utils.o

auth.o: _server/auth.cpp _server/auth.h _server/utils.h
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

text.o: _client/text.cpp _client/text.h
	$(CC) $(CFLAGS) -c _client/text.cpp -o text.o $(LDFLAGS)

helper.o: _client/helper.cpp _client/helper.h
	$(CC) $(CFLAGS) -c _client/helper.cpp -o helper.o $(LDFLAGS)

# Clean Rule
clean:
	rm -f *.o server client
