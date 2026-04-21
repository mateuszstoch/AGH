CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

all: main

main: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f main
