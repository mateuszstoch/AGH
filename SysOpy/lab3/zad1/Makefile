CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11

TARGET  = zad1

.PHONY: all clean run

all: $(TARGET)

$(TARGET): zad1.c
	$(CC) $(CFLAGS) -o $@ $<

# Przykładowe uruchomienie:
#   dx=0.000001 (dokładność), n=4 (maks. liczba procesów)
#   Dobierz dx tak, aby obliczenia trwały kilka sekund.
run: $(TARGET)
	./$(TARGET) 0.000001 4

clean:
	rm -f $(TARGET)
