/*
 * Zadanie 2 - Program 1 (master)
 *
 * Czyta od użytkownika przedział całkowania [a, b],
 * wysyła go przez FIFO "fifo_req" do programu 2,
 * odbiera wynik przez FIFO "fifo_res" i wyświetla go.
 *
 * FIFO-a są tworzone tutaj (jeśli jeszcze nie istnieją).
 *
 * Użycie: uruchom najpierw prog2 w tle, potem prog1
 *   ./prog2 &
 *   ./prog1
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

/* Nazwy potoków nazwanych */
#define FIFO_REQ "fifo_req"  /* prog1 -> prog2: przedział [a, b] i dx */
#define FIFO_RES "fifo_res"  /* prog2 -> prog1: wynik całkowania */

/* Struktura przesyłana do programu 2 */
typedef struct {
    double a;   /* lewy koniec przedziału */
    double b;   /* prawy koniec przedziału */
    double dx;  /* szerokość prostokąta */
} Request;

int main(void) {
    /* Tworzymy FIFO (mkfifo ignoruje błąd jeśli plik już istnieje – EEXIST) */
    mkfifo(FIFO_REQ, 0666);
    mkfifo(FIFO_RES, 0666);

    printf("=== Program 1 (master) ===\n");
    printf("Podaj przedział całkowania i dokładność.\n");

    Request req;
    printf("Podaj a (lewy koniec): ");
    if (scanf("%lf", &req.a) != 1) { fprintf(stderr, "Błąd wejścia\n"); return 1; }
    printf("Podaj b (prawy koniec): ");
    if (scanf("%lf", &req.b) != 1) { fprintf(stderr, "Błąd wejścia\n"); return 1; }
    printf("Podaj dx (szerokość prostokąta, np. 0.000001): ");
    if (scanf("%lf", &req.dx) != 1) { fprintf(stderr, "Błąd wejścia\n"); return 1; }

    printf("\nWysyłam do prog2: a=%.6f b=%.6f dx=%.9f\n", req.a, req.b, req.dx);

    /* Otwieramy FIFO do zapisu (blokuje do momentu otwarcia przez prog2) */
    int fd_req = open(FIFO_REQ, O_WRONLY);
    if (fd_req == -1) { perror("open FIFO_REQ"); return 1; }

    /* Wysyłamy strukturę z parametrami */
    if (write(fd_req, &req, sizeof(req)) != sizeof(req)) {
        perror("write FIFO_REQ");
        close(fd_req);
        return 1;
    }
    close(fd_req);

    /* Otwieramy FIFO do odczytu (blokuje do momentu wysłania przez prog2) */
    int fd_res = open(FIFO_RES, O_RDONLY);
    if (fd_res == -1) { perror("open FIFO_RES"); return 1; }

    double result = 0.0;
    if (read(fd_res, &result, sizeof(result)) != sizeof(result)) {
        perror("read FIFO_RES");
        close(fd_res);
        return 1;
    }
    close(fd_res);

    printf("Wynik całkowania: %.10f\n", result);

    return 0;
}
