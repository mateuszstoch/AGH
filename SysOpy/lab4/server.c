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


/* Informacje o podłączonych klientach */
typedef struct {
    int  active;                    /* czy slot jest zajęty        */
    mqd_t queue;                    /* deskryptor kolejki klienta  */
    char  queue_name[64];           /* nazwa kolejki (do logów)    */
} ClientInfo;

/* Globalna tablica klientów */
static ClientInfo clients[MAX_CLIENTS];
static int        client_count = 0;     /* ilu klientów łącznie    */
static mqd_t      server_mq;           /* deskryptor kolejki serwera */

/* Sprzątamy przy wyjściu - zamknij i usuń kolejkę */
static void cleanup(void) {
    mq_close(server_mq);
    mq_unlink(SERVER_QUEUE_NAME);

    /* Zamknij też kolejki wszystkich klientów */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            mq_close(clients[i].queue);
        }
    }
    printf("[serwer] Kolejka usunięta, żegnaj!\n");
}

/* Handler sygnału - żeby Ctrl+C sprzątało po sobie */
static void sig_handler(int sig) {
    (void)sig;
    cleanup();
    exit(0);
}

/* Roześlij wiadomość do wszystkich klientów POZA nadawcą */
static void broadcast(int sender_id, const char *text) {
    Message out;
    memset(&out, 0, sizeof(out));
    out.mtype     = MSG_CHAT;
    out.client_id = sender_id;
    snprintf(out.text, sizeof(out.text), "[klient %d]: %s", sender_id, text);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active)
            continue;
        /* Pomijamy nadawcę - nie chcemy echa własnej wiadomości */
        if (i == sender_id)
            continue;

        if (mq_send(clients[i].queue, (char *)&out, sizeof(out), 0) == -1) {
            /* Jeśli wysyłka padła - pewnie klient się rozłączył */
            perror("[serwer] mq_send do klienta");
        }
    }
}

int main(void) {
    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    /* Atrybuty kolejki - ograniczamy rozmiar wiadomości */
    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;                /* max 10 wiadomości w kolejce */
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0;

    /* Tworzymy kolejkę serwera - klienci będą tu pisać */
    server_mq = mq_open(SERVER_QUEUE_NAME,
                        O_CREAT | O_RDONLY,
                        0666, &attr);
    if (server_mq == (mqd_t)-1) {
        perror("[serwer] mq_open");
        exit(1);
    }

    printf("[serwer] Uruchomiony. Czekam na klientów...\n");
    printf("[serwer] Kolejka: %s\n", SERVER_QUEUE_NAME);

    /* Główna pętla serwera - czytamy wiadomości i reagujemy */
    Message msg;
    while (1) {
        ssize_t bytes = mq_receive(server_mq,
                                   (char *)&msg,
                                   sizeof(Message),
                                   NULL);
        if (bytes == -1) {
            if (errno == EINTR)
                continue;   /* przerwanie sygnałem - lecimy dalej */
            perror("[serwer] mq_receive");
            continue;
        }

        if (msg.mtype == MSG_INIT) {
            /* Nowy klient - sprawdzamy czy jest miejsce */
            if (client_count >= MAX_CLIENTS) {
                printf("[serwer] Brak miejsca dla nowego klienta!\n");
                continue;
            }

            /* Znajdź wolny slot w tablicy */
            int id = -1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i].active) {
                    id = i;
                    break;
                }
            }

            /* Otwieramy kolejkę klienta (klient już ją stworzył) */
            mqd_t cq = mq_open(msg.client_queue, O_WRONLY);
            if (cq == (mqd_t)-1) {
                perror("[serwer] mq_open klienta");
                continue;
            }

            /* Zapisz klienta w tablicy */
            clients[id].active = 1;
            clients[id].queue  = cq;
            strncpy(clients[id].queue_name, msg.client_queue,
                    sizeof(clients[id].queue_name) - 1);
            client_count++;

            printf("[serwer] Nowy klient! ID=%d, kolejka=%s\n",
                   id, msg.client_queue);

            /* Odeślij klientowi jego nowiutki identyfikator */
            Message reply;
            memset(&reply, 0, sizeof(reply));
            reply.mtype     = MSG_ASSIGN;
            reply.client_id = id;

            if (mq_send(cq, (char *)&reply, sizeof(reply), 0) == -1) {
                perror("[serwer] mq_send ASSIGN");
            }

        } else if (msg.mtype == MSG_CHAT) {
            /* Normalna wiadomość - przesyłamy dalej */
            int cid = msg.client_id;
            if (cid < 0 || cid >= MAX_CLIENTS || !clients[cid].active) {
                printf("[serwer] Wiadomość od nieznanego klienta %d, ignoruję\n",
                       cid);
                continue;
            }

            printf("[serwer] Klient %d: %s\n", cid, msg.text);
            broadcast(cid, msg.text);

        } else {
            printf("[serwer] Nieznany typ wiadomości: %ld\n", msg.mtype);
        }
    }

    cleanup();
    return 0;
}
