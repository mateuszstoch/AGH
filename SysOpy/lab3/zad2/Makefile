CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11

TARGETS = prog1 prog2

.PHONY: all clean run

all: $(TARGETS)

prog1: prog1.c
	$(CC) $(CFLAGS) -o $@ $<

prog2: prog2.c
	$(CC) $(CFLAGS) -o $@ $<

# Uruchamia prog2 w tle, czeka chwilę, potem uruchamia prog1 interaktywnie.
# Uwaga: w terminalu wpisz ręcznie wartości a, b, dx po uruchomieniu prog1.
run: all
	./prog2 & sleep 0.3 && ./prog1 ; wait

clean:
	rm -f $(TARGETS) fifo_req fifo_res
