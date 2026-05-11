CC      = gcc
CFLAGS  = -Wall -Wextra -g
# Na macOS kolejki POSIX wymagają -lrt, na Linuksie też (na macOS nie ma mq_*)
# Zakładamy Linuksa (AGH lab)
LIBS    = -lrt

.PHONY: all clean

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LIBS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LIBS)

clean:
	rm -f server client
	# Na wszelki wypadek usuń też kolejki (jeśli coś padło)
	-rm -f /dev/mqueue/chat_server /dev/mqueue/chat_client_*
