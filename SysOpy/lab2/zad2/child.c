#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

/* Globalny tryb ustawiony przez USR2 */
static int current_mode = -1;

/* Handler dla SIGUSR1 w trybie "handle" */
void signal_handler(int signo) {
    printf("[child] Wywołano handler dla sygnału %d\n", signo);
    fflush(stdout);
}

/*
 * Handler dla SIGUSR2 — używamy SA_SIGINFO żeby dostać siginfo_t
 * i z niego wyciągnąć wartość przesłaną przez sigqueue() (si_value.sival_int)
 * Ta wartość mówi nam który tryb ustawić dla SIGUSR1
 */
void usr2_handler(int signo, siginfo_t *info, void *context) {
    (void)context; /* nie używamy tego parametru, unikamy warning'a */
    printf("[child] Otrzymano sygnał USR2 (%d), wartość: %d\n", signo, info->si_value.sival_int);
    fflush(stdout);

    current_mode = info->si_value.sival_int;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    /* Ustawiamy reakcję na SIGUSR1 na podstawie wartości z sigqueue */
    switch (current_mode) {
        case 0: /* default */
            printf("[child] Wywołano funkcję 'sig_default()'\n");
            sa.sa_handler = SIG_DFL;
            sigaction(SIGUSR1, &sa, NULL);
            break;
        case 1: /* mask */
            printf("[child] Wywołano funkcję 'sig_mask()'\n");
            /* Blokadę nakładamy po powrocie z sigsuspend — patrz main() */
            break;
        case 2: /* ignore */
            printf("[child] Wywołano funkcję 'sig_ignore()'\n");
            sa.sa_handler = SIG_IGN;
            sigaction(SIGUSR1, &sa, NULL);
            break;
        case 3: /* handle */
            printf("[child] Wywołano funkcję 'sig_handle()'\n");
            sa.sa_handler = signal_handler;
            sigaction(SIGUSR1, &sa, NULL);
            break;
        default:
            fprintf(stderr, "[child] Nieznany tryb: %d\n", mode);
            break;
    }
}

/* Odblokowuje SIGUSR1 jeśli był zablokowany */
void sig_unblock() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main() {
    /* Rejestrujemy handler dla SIGUSR2 z flagą SA_SIGINFO
     * żeby handler dostawał dodatkowe info (siginfo_t) — w tym wartość z sigqueue */
    struct sigaction sa2;
    sa2.sa_sigaction = usr2_handler;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_SIGINFO; /* kluczowa flaga — bez niej nie dostaniemy si_value */
    sigaction(SIGUSR2, &sa2, NULL);

    /* Blokujemy SIGUSR2 zanim zaczniemy czekać — żeby sigsuspend działał poprawnie */
    sigset_t waitset, oldset;
    sigemptyset(&waitset);
    sigaddset(&waitset, SIGUSR2);
    sigprocmask(SIG_BLOCK, &waitset, &oldset);

    /* Informujemy rodzica że jesteśmy gotowi */
    printf("[child] PID: %d, czekam na USR2...\n", getpid());
    fflush(stdout);

    /* sigsuspend atomowo: odblokuje SIGUSR2 i usypia proces aż do sygnału */
    sigsuspend(&oldset);

    /*
     * Po powrocie z sigsuspend maska jest przywrócona do stanu sprzed wywołania
     * (tzn. tylko SIGUSR2 zablokowany). Jeśli tryb to "mask", musimy teraz
     * ponownie zablokować SIGUSR1 — handler w trakcie sigsuspend nie mógł
     * tego zrobić trwale, bo maska jest resetowana przy powrocie.
     */
    if (current_mode == 1) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        printf("[child] Zablokowano SIGUSR1 (tryb mask)\n");
        fflush(stdout);
    }

    /* Pętla główna — identyczna jak w zad1 */
    for (int i = 1; i <= 20; i++) {
        printf("[child] %d\n", i);
        fflush(stdout);

        if (i == 5 || i == 15) {
            printf("[child] Wysyłam sygnał USR1\n");
            fflush(stdout);
            raise(SIGUSR1);
        }

        if (i == 10) {
            sigset_t pending;
            sigpending(&pending);
            if (sigismember(&pending, SIGUSR1)) {
                printf("[child] Odblokowuję USR1\n");
                fflush(stdout);
                sig_unblock();
            }
        }

        sleep(1);
    }

    printf("[child] Pętla została wykonana w całości\n");
    return 0;
}
