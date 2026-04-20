#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"

/* Zdejmuje blokadę z SIGUSR1 — jeśli sygnał czekał, zostanie teraz dostarczony */
void sig_unblock(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    /* SIG_UNBLOCK = usuń podany zbiór z maski blokowania */
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
