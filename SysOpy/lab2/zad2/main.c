#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/* Mapujemy nazwę trybu na liczbę: 0=default, 1=mask, 2=ignore, 3=handle */
static int parse_mode(const char *arg) {
    if (strcmp(arg, "default") == 0) return 0;
    if (strcmp(arg, "mask")    == 0) return 1;
    if (strcmp(arg, "ignore")  == 0) return 2;
    if (strcmp(arg, "handle")  == 0) return 3;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Wywołanie: %s default|mask|ignore|handle\n", argv[0]);
        return 1;
    }

    int mode = parse_mode(argv[1]);
    if (mode < 0) {
        fprintf(stderr, "Nieznany argument: %s\n", argv[1]);
        return 1;
    }

    /* Tworzymy proces potomny */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Jesteśmy w procesie potomnym — uruchamiamy program child */
        execl("./child", "./child", NULL);
        perror("execl"); /* jeśli execl się nie udało */
        exit(1);
    }

    /* Jesteśmy w procesie rodzica */
    printf("[parent] Uruchomiono child z PID %d, tryb: %s (%d)\n", pid, argv[1], mode);
    fflush(stdout);

    /* Czekamy chwilę, żeby child zdążył zarejestrować handler SIGUSR2 */
    sleep(1);

    /* sigqueue() = kill() + możliwość przesłania dodatkowej wartości */
    union sigval sv;
    sv.sival_int = mode; /* przekazujemy numer trybu jako payload sygnału */

    if (sigqueue(pid, SIGUSR2, sv) < 0) {
        perror("sigqueue");
        return 1;
    }
    printf("[parent] Wysłano SIGUSR2 do child (tryb=%d)\n", mode);
    fflush(stdout);

    /* Czekamy aż child skończy działanie */
    int status;
    waitpid(pid, &status, 0);
    printf("[parent] Child zakończony, status=%d\n", WEXITSTATUS(status));
    return 0;
}
