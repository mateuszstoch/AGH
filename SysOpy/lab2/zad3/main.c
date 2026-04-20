/*
 * main.c – Zadanie 3
 *
 * Kompiluj z jedną z flag:
 *   -DUSE_STATIC   → linkowanie statyczne (libsignals_static.a)
 *   -DUSE_SHARED   → linkowanie dynamiczne (libsignals_shared.so przez linker)
 *   -DUSE_DYNAMIC  → ładowanie dynamiczne w trakcie działania programu (dlopen)
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#ifdef USE_DYNAMIC
  /* dlopen/dlsym/dlclose są w tym nagłówku */
  #include <dlfcn.h>
#else
  /* W trybach statycznym i shared funkcje są dostępne przez linker */
  #include "signal_handlers.h"
#endif

/* Pomocnicza lokalna wersja sig_unblock — używana tylko w trybie dynamicznym
 * gdy z jakiegoś powodu nie znajdziemy funkcji w bibliotece */
static void sig_unblock_local(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Wywołanie: %s default|mask|ignore|handle\n", argv[0]);
        return 1;
    }

#ifdef USE_DYNAMIC
    /* ---- Tryb: ładowanie dynamiczne przez dlopen() ---- */

    /* Otwieramy bibliotekę .so w czasie działania programu */
    void *handle = dlopen("./libsignals_dynamic.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return 1;
    }
    dlerror(); /* czyścimy ewentualny błąd przed wywołaniem dlsym */

    /* Typ wskaźnika na funkcję void bez argumentów */
    typedef void (*sig_fn_t)(void);
    sig_fn_t fn = NULL;

    /* Pobieramy adres odpowiedniej funkcji z biblioteki po nazwie */
    if (strcmp(argv[1], "default") == 0) {
        fn = (sig_fn_t)dlsym(handle, "sig_default");
    } else if (strcmp(argv[1], "mask") == 0) {
        fn = (sig_fn_t)dlsym(handle, "sig_mask");
    } else if (strcmp(argv[1], "ignore") == 0) {
        fn = (sig_fn_t)dlsym(handle, "sig_ignore");
    } else if (strcmp(argv[1], "handle") == 0) {
        fn = (sig_fn_t)dlsym(handle, "sig_handle");
    } else {
        fprintf(stderr, "Nieznany argument: %s\n", argv[1]);
        dlclose(handle);
        return 1;
    }

    /* Sprawdzamy czy dlsym nie zwrócił błędu */
    char *error = dlerror();
    if (error) {
        fprintf(stderr, "dlsym: %s\n", error);
        dlclose(handle);
        return 1;
    }

    fn(); /* wywołujemy znalezioną funkcję */

    /* Pobieramy też sig_unblock z biblioteki */
    typedef void (*unblock_fn_t)(void);
    unblock_fn_t unblock_fn = (unblock_fn_t)dlsym(handle, "sig_unblock");

#else
    /* ---- Tryb: statyczny lub shared (linkowany przez linker) ---- */
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
        return 1;
    }
#endif

    /* Pętla główna — taka sama jak w zad1 */
    for (int i = 1; i <= 20; i++) {
        printf("%d\n", i);
        fflush(stdout);

        if (i == 5 || i == 15) {
            printf("Wysyłam sygnał USR1\n");
            fflush(stdout);
            raise(SIGUSR1);
        }

        if (i == 10) {
            sigset_t pending;
            sigpending(&pending);
            if (sigismember(&pending, SIGUSR1)) {
                printf("Odblokowuję USR1\n");
                fflush(stdout);
#ifdef USE_DYNAMIC
                /* W trybie dynamicznym używamy funkcji z biblioteki lub lokalnej kopii */
                if (unblock_fn) unblock_fn();
                else sig_unblock_local();
#else
                sig_unblock();
#endif
            }
        }

        sleep(1);
    }

    printf("Pętla została wykonana w całości\n");

#ifdef USE_DYNAMIC
    dlclose(handle); /* zwalniamy bibliotekę */
#endif

    return 0;
}
