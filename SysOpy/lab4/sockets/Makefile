CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11

.PHONY: all clean

all: server client

server: server.c chat.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c chat.h
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
