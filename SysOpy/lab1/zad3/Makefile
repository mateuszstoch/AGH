CC = gcc
RM = rm -f
CFLAGS = -Wall -O3
LDFLAGS = -lm

# Znajduje wszystkie pliki .c w folderze
SOURCES = $(wildcard *.c)
# Tworzy listę obiektów (plik.o)
OBJECTS = $(SOURCES:.c=.o)
# Tworzy listę plików wykonywalnych (plik bez rozszerzenia)
EXECS = $(SOURCES:.c=)

.PHONY: all clean


all: $(EXECS)


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


%: %.o
	$(CC) $< $(LDFLAGS) -o $@

clean:
	@echo "Czyszczenie..."
	$(RM) *.o $(EXECS)