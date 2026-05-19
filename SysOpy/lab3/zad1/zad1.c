/*
 * Zadanie 1 - Całkowanie numeryczne metodą prostokątów Riemanna
 *
 * Całkujemy f(x) = 4/(x^2 + 1) na [0, 1] => wynik powinien dążyć do pi
 *
 * Użycie: ./zad1 <szerokosc_prostokata> <n_max_procesow>
 *
 * Program dla każdego k = 1..n uruchamia k procesów potomnych.
 * Każdy potomek liczy swój fragment przedziału i wysyła wynik
 * przez swój własny potok nienazwany (pipe) do rodzica.
 * Rodzic sumuje wyniki i wyświetla czas wykonania.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

/* Funkcja podcałkowa: całka z 4/(x^2+1) dx na [0,1] = pi */
static double f(double x) {
    return 4.0 / (x * x + 1.0);
}

/*
 * Całkowanie metodą prostokątów (punkt środkowy każdego prostokąta)
 * na przedziale [start, end] z krokiem dx.
 */
static double integrate(double start, double end, double dx) {
    double sum = 0.0;
    /* iterujemy po środkach prostokątów */
    for (double x = start + dx / 2.0; x < end; x += dx) {
        sum += f(x) * dx;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <szerokosc_prostokata> <n_max_procesow>\n", argv[0]);
        return 1;
    }

    double dx = atof(argv[1]);
    int n    = atoi(argv[2]);

    if (dx <= 0 || n <= 0) {
        fprintf(stderr, "Błąd: dx i n muszą być dodatnie.\n");
        return 1;
    }

    printf("%-5s %-20s %-15s\n", "k", "wynik", "czas [s]");
    printf("%-5s %-20s %-15s\n", "---", "--------------------", "---------------");

    /* Zewnętrzna pętla: k = liczba procesów potomnych */
    for (int k = 1; k <= n; k++) {

        /* Tworzymy k potoków – każdy potomek ma swój własny deskryptor do zapisu */
        int (*pipes)[2] = malloc(k * sizeof(*pipes));
        if (!pipes) {
            perror("malloc");
            return 1;
        }

        for (int i = 0; i < k; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                free(pipes);
                return 1;
            }
        }

        /* Mierzymy czas od fork do zebrania wszystkich wyników */
        struct timespec ts_start, ts_end;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

        /* Tworzymy k procesów potomnych */
        for (int i = 0; i < k; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                free(pipes);
                return 1;
            }

            if (pid == 0) {
                /* === POTOMEK i === */

                /* Zamykamy deskryptory odczytu wszystkich potoków – potomek tylko pisze */
                for (int j = 0; j < k; j++) {
                    close(pipes[j][0]);
                }
                /* Zamykamy też deskryptory zapisu innych potoków */
                for (int j = 0; j < k; j++) {
                    if (j != i) close(pipes[j][1]);
                }

                /* Obliczamy fragment przedziału dla tego procesu */
                double seg = 1.0 / k;          /* długość fragmentu */
                double start = i * seg;
                double end   = (i + 1) * seg;

                double result = integrate(start, end, dx);

                /* Wysyłamy wynik jako binary double przez potok */
                write(pipes[i][1], &result, sizeof(double));
                close(pipes[i][1]);
                exit(0);
            }
            /* === RODZIC === zamknie deskryptory zapisu po pętli */
        }

        /* Rodzic zamyka wszystkie deskryptory zapisu (nie pisze) */
        for (int i = 0; i < k; i++) {
            close(pipes[i][1]);
        }

        /* Zbieramy wyniki od każdego potomka */
        double total = 0.0;
        for (int i = 0; i < k; i++) {
            double partial = 0.0;
            ssize_t bytes = read(pipes[i][0], &partial, sizeof(double));
            if (bytes != sizeof(double)) {
                fprintf(stderr, "Błąd odczytu z potoku %d\n", i);
            }
            close(pipes[i][0]);
            total += partial;
        }

        /* Czekamy na zakończenie wszystkich potomków */
        for (int i = 0; i < k; i++) {
            wait(NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        double elapsed = (ts_end.tv_sec - ts_start.tv_sec)
                       + (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

        printf("%-5d %-20.10f %-15.6f\n", k, total, elapsed);
        fflush(stdout);

        free(pipes);
    }

    return 0;
}
