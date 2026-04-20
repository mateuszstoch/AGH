#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"

/* Blokuje SIGUSR1 — sygnał nie jest tracony, tylko czeka w kolejce */
void sig_mask(void) {
    printf("Wywołano funkcję 'sig_mask()'\n");

    sigset_t mask;
    sigemptyset(&mask);          /* tworzymy pustą maskę */
    sigaddset(&mask, SIGUSR1);   /* dodajemy SIGUSR1 do maski */

    /* SIG_BLOCK = do obecnej maski blokowania dodaj podany zbiór */
    sigprocmask(SIG_BLOCK, &mask, NULL);
}
