#include <stdio.h>
#include <signal.h>
#include "signal_handlers.h"

/* Przywraca domyślną reakcję systemu na SIGUSR1 */
void sig_default(void) {
    printf("Wywołano funkcję 'sig_default()'\n");

    struct sigaction sa;
    sa.sa_handler = SIG_DFL;   /* SIG_DFL = działanie domyślne (zwykle zabicie procesu) */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}
