#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "chat.h"


/* Globale używane przez handler sygnału */
static mqd_t  client_mq;
static char   my_queue_name[64];
static pid_t  child_pid = -1;

/* Sprzątamy - ubijamy dziecko i usuwamy własną kolejkę */
static void cleanup(void) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);   /* koniec dla procesu odbierającego */
    }
    mq_close(client_mq);
    mq_unlink(my_queue_name);
}

static void sig_handler(int sig) {
    (void)sig;
    cleanup();
    exit(0);
}

/*
 * Pętla dziecka - tylko odbiera i drukuje.
 * Blokuje się na mq_receive, więc nie żre CPU.
 */
static void receiver_loop(mqd_t mq) {
    Message msg;
    printf("[odbiór] Gotowy na wiadomości od innych klientów.\n");

    while (1) {
        ssize_t bytes = mq_receive(mq, (char *)&msg, sizeof(Message), NULL);
        if (bytes == -1) {
            if (errno == EINTR)
                continue;
            perror("[odbiór] mq_receive");
            continue;
        }

        /* Wypisz na standardowe wyjście - MSG_CHAT ma już sformatowany tekst */
        if (msg.mtype == MSG_CHAT) {
            printf("\n%s\n", msg.text);
            fflush(stdout);
        }
    }
}

int main(void) {
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    /* Unikalna nazwa kolejki - używamy PID żeby kilka klientów mogło działać naraz */
    snprintf(my_queue_name, sizeof(my_queue_name), "/chat_client_%d", getpid());

    /* Atrybuty kolejki - takie same jak po stronie serwera */
    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0;

    /* Tworzymy NASZĄ kolejkę - będziemy tu dostawać wiadomości od serwera */
    client_mq = mq_open(my_queue_name,
                        O_CREAT | O_RDWR,
                        0666, &attr);
    if (client_mq == (mqd_t)-1) {
        perror("[klient] mq_open własna kolejka");
        exit(1);
    }

    printf("[klient] Własna kolejka: %s\n", my_queue_name);

    /* Otwieramy kolejkę serwera żeby wysłać INIT */
    mqd_t server_mq = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (server_mq == (mqd_t)-1) {
        perror("[klient] mq_open serwer (czy serwer jest uruchomiony?)");
        cleanup();
        exit(1);
    }

    /* Przygotuj wiadomość INIT - powiedz serwerowi skąd pisać z powrotem */
    Message init_msg;
    memset(&init_msg, 0, sizeof(init_msg));
    init_msg.mtype     = MSG_INIT;
    init_msg.client_id = 0;    /* jeszcze nie mamy ID */
    strncpy(init_msg.client_queue, my_queue_name,
            sizeof(init_msg.client_queue) - 1);

    if (mq_send(server_mq, (char *)&init_msg, sizeof(init_msg), 0) == -1) {
        perror("[klient] mq_send INIT");
        cleanup();
        exit(1);
    }

    printf("[klient] INIT wysłany, czekam na ID od serwera...\n");

    /* Czekamy na odpowiedź serwera z przydzielonym ID */
    Message reply;
    if (mq_receive(client_mq, (char *)&reply, sizeof(Message), NULL) == -1) {
        perror("[klient] mq_receive ASSIGN");
        cleanup();
        exit(1);
    }

    int my_id = reply.client_id;
    printf("[klient] Połączono! Mój ID: %d\n", my_id);
    printf("[klient] Pisz wiadomości (Ctrl+C żeby wyjść):\n");

    /*
     * Fork - dziecko zajmuje się odbieraniem wiadomości od serwera.
     * Rodzic czyta ze stdin i wysyła do serwera.
     * Klasyczny podział pracy.
     */
    child_pid = fork();
    if (child_pid == -1) {
        perror("[klient] fork");
        cleanup();
        exit(1);
    }

    if (child_pid == 0) {
        /* === DZIECKO - tylko odbiera i drukuje === */
        receiver_loop(client_mq);
        /* nigdy tu nie dotrzemy normalnie */
        exit(0);
    }

    /* === RODZIC - czyta stdin i wysyła do serwera === */
    char line[MAX_MSG_SIZE];
    while (1) {
        /* fgets blokuje do czasu wpisania linii */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            /* EOF - użytkownik zamknął stdin, wychodzimy */
            break;
        }

        /* Usuń newline na końcu */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        /* Pusta linia? Ignorujemy */
        if (strlen(line) == 0)
            continue;

        /* Zapakuj wiadomość i wyślij do serwera */
        Message out;
        memset(&out, 0, sizeof(out));
        out.mtype     = MSG_CHAT;
        out.client_id = my_id;
        strncpy(out.text, line, sizeof(out.text) - 1);

        if (mq_send(server_mq, (char *)&out, sizeof(out), 0) == -1) {
            perror("[klient] mq_send wiadomość");
        }
    }

    /* Koniec - posprzątaj */
    cleanup();
    mq_close(server_mq);
    return 0;
}
