/*
 * Zadanie 2 - Program 2 (worker)
 *
 * Czeka na dane z FIFO "fifo_req" (przedział [a, b] i dx),
 * oblicza całkę metodą prostokątów Riemanna,
 * i odsyła wynik przez FIFO "fifo_res" do programu 1.
 *
 * Użycie: uruchom przed prog1
 *   ./prog2 &
 *   ./prog1
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_REQ "fifo_req"
#define FIFO_RES "fifo_res"

/* Musi być identyczna jak w prog1.c */
typedef struct {
    double a;
    double b;
    double dx;
} Request;

/* Funkcja podcałkowa: 4/(x^2 + 1) */
static double f(double x) {
    return 4.0 / (x * x + 1.0);
}

/* Całkowanie metodą środkowego prostokąta na [a, b] z krokiem dx */
static double integrate(double a, double b, double dx) {
    double sum = 0.0;
    for (double x = a + dx / 2.0; x < b; x += dx) {
        sum += f(x) * dx;
    }
    return sum;
}

int main(void) {
    /* Upewniamy się, że FIFO istnieją (prog1 też je tworzy, kolejność nieistotna) */
    mkfifo(FIFO_REQ, 0666);
    mkfifo(FIFO_RES, 0666);

    printf("=== Program 2 (worker) ===\n");
    printf("Czekam na żądanie od prog1...\n");

    /* Otwieramy FIFO żądania do odczytu – blokuje do czasu otwarcia przez prog1 */
    int fd_req = open(FIFO_REQ, O_RDONLY);
    if (fd_req == -1) { perror("open FIFO_REQ"); return 1; }

    Request req;
    if (read(fd_req, &req, sizeof(req)) != sizeof(req)) {
        perror("read FIFO_REQ");
        close(fd_req);
        return 1;
    }
    close(fd_req);

    printf("Otrzymałem: a=%.6f b=%.6f dx=%.9f\n", req.a, req.b, req.dx);
    printf("Liczę całkę...\n");

    double result = integrate(req.a, req.b, req.dx);
    printf("Wynik: %.10f, odsyłam do prog1...\n", result);

    /* Odsyłamy wynik przez drugie FIFO */
    int fd_res = open(FIFO_RES, O_WRONLY);
    if (fd_res == -1) { perror("open FIFO_RES"); return 1; }

    if (write(fd_res, &result, sizeof(result)) != sizeof(result)) {
        perror("write FIFO_RES");
        close(fd_res);
        return 1;
    }
    close(fd_res);

    printf("Zakończono.\n");
    return 0;
}
