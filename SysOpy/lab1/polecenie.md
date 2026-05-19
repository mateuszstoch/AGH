Procesy
Przygotowanie do ćwiczenia
Zainstaluj IDE, w którym będziesz pracować przez resztę laboratoriów — VS Code, Vim itp.
Wykonaj komendę $ mkdir zad1 zad2 zad3.
Zadanie 1 (25%)
Utwórz plik zad1/main.c zawierający program, który:
Zawiera zmienną globalną zmiennaGlobalna o ustalonej, przez Ciebie, wartości początkowej.
Tworzy N procesów potomnych za pomocą funkcji fork(), gdzie N jest argumentem wywołania programu — przykład: $ ./main 3.
Każdy proces potomny:
Inkrementuje zmienną zmiennaGlobalna.
Wypisuje M razy tekst "Potomek (PID)", gdzie PID jest identyfikatorem bieżącego procesu, czyli potomnego; wartość M jest zdefiniowana, w kodzie źródłowym, za pomocą dyrektywy #define preprocesora C.
Po każdym wypisaniu wywołuje funkcję sleep(0.25).
Gdy wszystkie procesy potomne wypiszą swoje teksty i zakończą działanie, to proces rodzicielski ma wypisać "Rodzic  (PID) zmiennaGlobalna=wartość", gdzie:
PID
jest identyfikatorem bieżącego procesu, czyli rodzicielskiego.
wartość
jest aktualną wartościa zmiennej zmiennaGlobalna.
Sprawdź, czy:
Procesy potomne wypisują M komunikatów naraz (spójny blok komunikatów), czy komunikaty z wielu procesów są przemieszane?
Zmienna zmiennaGlobalna jest wspólna dla wszystkich procesów potomnych?
Zastąp wywołanie fork() wywołaniem funkcji vfork() i sprawdź to, co powyżej.
Przykład działania dla N równego 3, a M równego 5:

$ ./main 3
Potomek (PID: 14316)
Potomek (PID: 14317)
Potomek (PID: 14316)
Potomek (PID: 14316)
Potomek (PID: 14317)
Potomek (PID: 14316)
Potomek (PID: 14317)
Potomek (PID: 14318)
Potomek (PID: 14316)
Potomek (PID: 14317)
Potomek (PID: 14317)
Potomek (PID: 14318)
Potomek (PID: 14318)
Potomek (PID: 14318)
Potomek (PID: 14318)
Rodzic  (PID: 14315) zmiennaGlobalna = 🤔
Visual Studio Code posiada rozszerzenie ułatwiające tworzenie programów w C/C+.

Zadanie 2 (25%)
Utwórz zmodyfikowaną wersję programu z zadania 1:
Utwórz plik zad2/child.c, zawierający kod źródłowy programu, który:
Nie zawiera obsługi zmiennej zmiennaGlobalna.
Wypisuje M razy tekst "Potomek (PID)", gdzie PID jest identyfikatorem bieżącego procesu, a wartość M nie jest już nazwą makra preprocesora lecz argumentem wywołania programu.
Po każdym wypisaniu wywołuje funkcję sleep(0.25).
Przykład działania:
$ ./child 5
Potomek (PID: 17697)
Potomek (PID: 17697)
Potomek (PID: 17697)
Potomek (PID: 17697)
Potomek (PID: 17697)   
Utwórz plik zad2/main.c zawierający program, który:
Nie zawiera obsługi zmiennej zmiennaGlobalna.
Odczytuje z linii komend argumenty wywołania — wartości N oraz M.
Tworzy N procesów potomnych.
Każdy z tych procesów wykonuje program child z argumentem M. Przykład działania dla N równego 3, a M równego 2:
$ ./main 3 2
Potomek (PID: 26657)
Potomek (PID: 26658)
Potomek (PID: 26657)
Potomek (PID: 26658)
Potomek (PID: 26656)
Potomek (PID: 26656)
Rodzic  (PID: 26655)
Utwórz plik zad2/Makefile zawierający następującą treść:
#
# Minimal Makefile which compiles multiple C files into individual executables.
#
#
# - Sarah Mount, November 2011
#
CC=gcc
RM=rm
CFLAGS=-c -Wall -O3
LDFLAGS=-lm
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECS=$(SOURCES:%.c=%)
.PHONY: all
all: $(OBJECTS) $(EXECS)
.c.o:
    $(CC) $(CFLAGS) $< -o $@
.o.: 
    $(CC) $^ $(LDFLAGS) -o $@
.PHONY: clean
clean:
    -@ $(RM) *.o 
    -@ $(RM) $(EXECS)        
Źródło: https://gist.github.com/snim2/1419118.
Uruchom kompilację komendą make.
Visual Studio Code posiada rozszerzenie ułatwiające pracę z plikami Makefile.

Zadanie 3 (50%)
Utwórz zmodyfikowaną wersję programu z zadania 2:
W katalogu zad3 wykonaj następujące komendy:
$ cp ../zad2/*.c .
$ ln -s ../zad2/Makefile .
Utwórz plik nagłówkowy definitions.h i przenieś do niego wszystkie wspólne (dla main.c i child.c) deklaracje zmiennych oraz dyrektywy #include preprocesora C.
Umieść, na początku każdego z plików .c, dyrektywę #include "definitions.h" i sprawdź, czy programy wciąż się kompilują.
Dopisz, do pliku nagłówkowego, makro stałe zawierające definicję nazwy pliku tekstowego — przykładowa nazwa to "output.txt".
W pliku main.c wstaw wywołanie funkcji systemowej, która usuwa powyższy plik tekstowy.
Zmodyfikuj zawartość pliku child.c:
Spowoduj, aby komunikaty "Potomek (PID)" zapisywały się do pliku tekstowego (przy użyciu funkcji biblioteki standardowej języka C), a nie były wypisywane na ekranie; ponadto wymuś, po każdorazowym zapisie danych (do pliku), natychmiastowy zapis danych znajdujących się w buforze wyjściowym.
Sprawdź, czy, każdorazowo, procesy potomne zapisują komunikaty naraz (spójny blok komunikatów), czy też komunikaty z wielu procesów są przemieszane?
Jeżeli są przemieszane, to spowoduj, aby w danym czasie tylko jeden proces miał możliwość zapisu danych do pliku (blokada na wyłączność); pozostałe mają czekać, aż blokada zostanie zdjęta.