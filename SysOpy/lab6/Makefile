CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -pthread
LDFLAGS = -lm

# Nazwy naszych plików wykonywalnych
TARGETS = program1 program2 program3

all: $(TARGETS)

# Budowanie programu 1
program1: program1.c common.c common.h
	$(CC) $(CFLAGS) program1.c common.c -o program1 $(LDFLAGS)

# Budowanie programu 2
program2: program2.c common.c common.h
	$(CC) $(CFLAGS) program2.c common.c -o program2 $(LDFLAGS)

# Budowanie programu 3
program3: program3.c common.c common.h
	$(CC) $(CFLAGS) program3.c common.c -o program3 $(LDFLAGS)

# Czyszczenie śmieci po kompilacji i działaniu programów
clean:
	rm -f $(TARGETS) *.o
	rm -f left_*.jpg right_*.jpg
	rm -f robot_pose.log report.txt
	@echo "Wyczyszczono katalog ze śmieci i plików wykonywalnych."

.PHONY: all clean
