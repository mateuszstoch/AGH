CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -D_DARWIN_C_SOURCE

all: main child

main: main.c
	$(CC) $(CFLAGS) -o main main.c

child: child.c
	$(CC) $(CFLAGS) -o child child.c

clean:
	rm -f main child
