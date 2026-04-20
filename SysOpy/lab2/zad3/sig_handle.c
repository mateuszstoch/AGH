#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"

/* Nasz własny handler — wypisuje numer obsługiwanego sygnału */
static void signal_handler(int signo) {
    printf("Wywołano handler dla sygnału %d\n", signo);
}

/* Podpina nasz handler pod SIGUSR1 */
void sig_handle(void) {
    printf("Wywołano funkcję 'sig_handle()'\n");

    struct sigaction sa;
    sa.sa_handler = signal_handler; /* wskaźnik na naszą funkcję */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}
