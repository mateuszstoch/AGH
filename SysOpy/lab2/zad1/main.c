#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

/* Ten handler jest używany w trybie "handle" — wypisuje numer sygnału */
void signal_handler(int signo) {
    printf("Wywołano handler dla sygnału %d\n", signo);
}

/* Przywraca domyślne zachowanie systemu dla SIGUSR1 (zwykle zabija proces) */
void sig_default() {
    printf("Wywołano funkcję 'sig_default()'\n");

    struct sigaction sa;
    sa.sa_handler = SIG_DFL;   /* SIG_DFL = domyślna reakcja systemu */
    sigemptyset(&sa.sa_mask);  /* żadne inne sygnały nie są blokowane podczas obsługi */
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

/* Maskuje (blokuje) SIGUSR1 — sygnał nie jest ignorowany, tylko tymczasowo wstrzymany */
void sig_mask() {
    printf("Wywołano funkcję 'sig_mask()'\n");

    sigset_t mask;
    sigemptyset(&mask);          /* zerujemy maskę */
    sigaddset(&mask, SIGUSR1);   /* dodajemy SIGUSR1 do maski */
    sigprocmask(SIG_BLOCK, &mask, NULL); /* blokujemy ten sygnał w procesie */
}

/* Ustawia ignorowanie SIGUSR1 — sygnał jest po prostu wyrzucany */
void sig_ignore() {
    printf("Wywołano funkcję 'sig_ignore()'\n");

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;   /* SIG_IGN = ignoruj sygnał */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

/* Podpina nasz własny handler pod SIGUSR1 */
void sig_handle() {
    printf("Wywołano funkcję 'sig_handle()'\n");

    struct sigaction sa;
    sa.sa_handler = signal_handler; /* nasza funkcja obsługi */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

/* Odblokowanie SIGUSR1 — używane gdy sygnał jest zablokowany (tryb mask) */
void sig_unblock() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask, NULL); /* zdejmujemy blokadę */
}

int main(int argc, char *argv[]) {
    /* Sprawdzamy czy podano jeden argument */
    if (argc != 2) {
        fprintf(stderr, "Wywołanie: %s default|mask|ignore|handle\n", argv[0]);
        return 1;
    }

    /* Ustawiamy reakcję na SIGUSR1 zgodnie z argumentem */
    if (strcmp(argv[1], "default") == 0) {
        sig_default();
    } else if (strcmp(argv[1], "mask") == 0) {
        sig_mask();
    } else if (strcmp(argv[1], "ignore") == 0) {
        sig_ignore();
    } else if (strcmp(argv[1], "handle") == 0) {
        sig_handle();
    } else {
        fprintf(stderr, "Nieznany argument: %s\n", argv[1]);
        fprintf(stderr, "Wywołanie: %s default|mask|ignore|handle\n", argv[0]);
        return 1;
    }

    /* Główna pętla — liczymy od 1 do 20, co sekundę */
    for (int i = 1; i <= 20; i++) {
        printf("%d\n", i);
        fflush(stdout); /* wymuszamy wypisanie od razu, nie z bufora */

        /* Na i=5 i i=15 wysyłamy sygnał do samego siebie */
        if (i == 5 || i == 15) {
            printf("Wysyłam sygnał USR1\n");
            fflush(stdout);
            raise(SIGUSR1); /* raise() = kill(getpid(), sygnał) */
        }

        /* Na i=10 sprawdzamy czy SIGUSR1 jest zablokowany i czeka */
        if (i == 10) {
            sigset_t pending;
            sigpending(&pending); /* pobiera zbiór sygnałów oczekujących */

            if (sigismember(&pending, SIGUSR1)) {
                printf("Odblokowuję USR1\n");
                fflush(stdout);
                sig_unblock(); /* odblokowujemy — sygnał zostanie wtedy dostarczony */
            }
        }

        sleep(1);
    }

    printf("Pętla została wykonana w całości\n");
    return 0;
}
