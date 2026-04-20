#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"

/* Ustawia ignorowanie SIGUSR1 — sygnał jest po prostu wyrzucany przez jądro */
void sig_ignore(void) {
    printf("Wywołano funkcję 'sig_ignore()'\n");

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;   /* SIG_IGN = ignoruj, nic nie rób */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}
