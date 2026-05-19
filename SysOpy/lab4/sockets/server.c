/*
 * server.c - Serwer czatu (TCP, POSIX sockets)
 *
 * Uruchomienie: ./server <port>
 *
 * Architektura: select() - jeden proces, brak wątków.
 * Serwer obserwuje jednocześnie gniazdo nasłuchujące i wszystkie
 * otwarte połączenia klientów. Co ALIVE_INTERVAL sekund rozsyła ping
 * i usuwa tych, którzy nie odpisali w ciągu ALIVE_TIMEOUT sekund.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.h"

/* Co ile sekund wysyłamy ALIVE ping do klientów */
#define ALIVE_INTERVAL  10
/* Po ilu sekundach bez odpowiedzi uznajemy klienta za martwego */
#define ALIVE_TIMEOUT   5

/* Informacje o jednym podłączonym kliencie */
typedef struct {
    int   fd;                   /* deskryptor połączenia (-1 = wolny slot) */
    char  name[MAX_NAME_LEN];   /* nick                                    */
    int   alive_pending;        /* 1 = wysłaliśmy ping, czekamy na pong    */
    time_t ping_sent_at;        /* kiedy wysłano ostatni ping              */
} ClientInfo;

static ClientInfo clients[MAX_CLIENTS];
static int        listen_fd = -1;   /* gniazdo nasłuchujące */
static int        client_count = 0;

/* Zamknij połączenie z klientem i zwolnij slot */
static void remove_client(int idx) {
    if (clients[idx].fd < 0) return;
    printf("[serwer] Klient '%s' (fd=%d) rozłączony\n",
           clients[idx].name, clients[idx].fd);
    close(clients[idx].fd);
    clients[idx].fd = -1;
    clients[idx].name[0] = '\0';
    clients[idx].alive_pending = 0;
    client_count--;
}

/* Wyślij pakiet do wszystkich aktywnych klientów POZA sender_idx */
static void broadcast(int sender_idx, const Packet *pkt) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd < 0) continue;
        if (i == sender_idx) continue;
        if (send_packet(clients[i].fd, pkt) < 0)
            remove_client(i);
    }
}

/* Sprzątamy przy wyjściu */
static void cleanup(void) {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i].fd >= 0)
            close(clients[i].fd);
    if (listen_fd >= 0)
        close(listen_fd);
    printf("[serwer] Do widzenia!\n");
}

static void sig_handler(int sig) {
    (void)sig;
    cleanup();
    exit(0);
}

/* Obsługa pakietu LIST - zbuduj odpowiedź z listą aktywnych nicków */
static void handle_list(int sender_idx) {
    Packet reply;
    memset(&reply, 0, sizeof(reply));
    reply.type = PKT_MSG;
    snprintf(reply.sender, sizeof(reply.sender), "serwer");

    /* Zbuduj listę: "ID: nick\n" dla każdego aktywnego klienta */
    char *buf  = reply.text;
    int   left = (int)sizeof(reply.text) - 1;
    for (int i = 0; i < MAX_CLIENTS && left > 0; i++) {
        if (clients[i].fd < 0) continue;
        int written = snprintf(buf, (size_t)left, "%d: %s\n", i, clients[i].name);
        if (written < 0) break;
        buf  += written; 
        left -= written;
    }

    send_packet(clients[sender_idx].fd, &reply);
}

/* Obsługa PKT_2ALL - dodaj datę i broadcast */
static void handle_2all(int sender_idx, const Packet *in) {
    Packet out;
    memset(&out, 0, sizeof(out));
    out.type = PKT_MSG;
    snprintf(out.sender, sizeof(out.sender), "%s", clients[sender_idx].name);

    /* Pobierz aktualną datę/godzinę i sklej z treścią */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);
    snprintf(out.text, sizeof(out.text), "[%s] %.*s",
             ts, (int)(sizeof(out.text) - 40), in->text);

    broadcast(sender_idx, &out);
    printf("[serwer] 2ALL od '%s': %s\n", clients[sender_idx].name, in->text);
}

/* Obsługa PKT_2ONE - wyślij tylko do wskazanego klienta */
static void handle_2one(int sender_idx, const Packet *in) {
    int target = (int)in->target_id;

    /* Walidacja: czy target istnieje i nie jest nadawcą */
    if (target < 0 || target >= MAX_CLIENTS || clients[target].fd < 0) {
        Packet err;
        memset(&err, 0, sizeof(err));
        err.type = PKT_MSG;
        snprintf(err.sender, sizeof(err.sender), "serwer");
        snprintf(err.text,   sizeof(err.text),   "Klient %d nie istnieje.", target);
        send_packet(clients[sender_idx].fd, &err);
        return;
    }

    Packet out;
    memset(&out, 0, sizeof(out));
    out.type = PKT_MSG;
    snprintf(out.sender, sizeof(out.sender), "%s", clients[sender_idx].name);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);
    snprintf(out.text, sizeof(out.text), "[prywatna %s] %.*s",
             ts, (int)(sizeof(out.text) - 60), in->text);

    send_packet(clients[target].fd, &out);
    printf("[serwer] 2ONE od '%s' do '%s': %s\n",
           clients[sender_idx].name, clients[target].name, in->text);
}

/* Wyślij ALIVE ping do wszystkich aktywnych klientów */
static void send_alive_pings(void) {
    Packet ping;
    memset(&ping, 0, sizeof(ping));
    ping.type = PKT_ALIVE;
    snprintf(ping.sender, sizeof(ping.sender), "serwer");

    time_t now = time(NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd < 0) continue;
        clients[i].alive_pending = 1;
        clients[i].ping_sent_at  = now;
        send_packet(clients[i].fd, &ping);
    }
}

/* Sprawdź czy ktoś przekroczył timeout ALIVE i go wyrzuć */
static void check_alive_timeouts(void) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd < 0)              continue;
        if (!clients[i].alive_pending)      continue;
        if (now - clients[i].ping_sent_at < ALIVE_TIMEOUT) continue;

        printf("[serwer] Klient '%s' nie odpowiedział na ping - usuwam.\n",
               clients[i].name);
        remove_client(i);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Nieprawidłowy port: %s\n", argv[1]);
        return 1;
    }

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    /* Inicjalizuj tablicę klientów - fd=-1 oznacza wolny slot */
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i].fd = -1;

    /* Tworzymy gniazdo nasłuchujące TCP */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    /* SO_REUSEADDR - żeby po restarcie serwera port był od razu dostępny */
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); return 1;
    }
    if (listen(listen_fd, 8) < 0) {
        perror("listen"); close(listen_fd); return 1;
    }

    printf("[serwer] Nasłuchuję na porcie %d...\n", port);

    time_t last_ping = time(NULL);

    /* Główna pętla: select() na gnieździe nasłuchującym + wszystkich klientach */
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        int maxfd = listen_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd < 0) continue;
            FD_SET(clients[i].fd, &rfds);
            if (clients[i].fd > maxfd) maxfd = clients[i].fd;
        }

        /* Timeout select = 1s żeby móc regularnie sprawdzać alive */
        struct timeval tv = {1, 0};
        int ready = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("select"); break;
        }

        /* Czas na ping? */
        time_t now = time(NULL);
        if (now - last_ping >= ALIVE_INTERVAL) {
            send_alive_pings();
            last_ping = now;
        }
        check_alive_timeouts();

        if (ready == 0) continue;   /* timeout bez zdarzeń */

        /* Nowe połączenie przychodzące */
        if (FD_ISSET(listen_fd, &rfds)) {
            struct sockaddr_in caddr;
            socklen_t clen = sizeof(caddr);
            int newfd = accept(listen_fd, (struct sockaddr *)&caddr, &clen);
            if (newfd < 0) {
                perror("accept");
            } else if (client_count >= MAX_CLIENTS) {
                /* Brak miejsca - grzecznie odmawiamy */
                printf("[serwer] Brak miejsca dla nowego klienta.\n");
                close(newfd);
            } else {
                /* Szukamy wolnego slotu */
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd >= 0) continue;
                    clients[i].fd = newfd;
                    clients[i].alive_pending = 0;
                    clients[i].name[0] = '\0';
                    client_count++;
                    printf("[serwer] Nowe połączenie fd=%d od %s:%d (slot %d)\n",
                           newfd, inet_ntoa(caddr.sin_addr),
                           ntohs(caddr.sin_port), i);
                    break;
                }
            }
        }

        /* Dane od istniejących klientów */
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd < 0) continue;
            if (!FD_ISSET(clients[i].fd, &rfds)) continue;

            Packet pkt;
            if (recv_packet(clients[i].fd, &pkt) < 0) {
                /* Połączenie zerwane bez STOP - sprzątamy */
                remove_client(i);
                continue;
            }

            switch (pkt.type) {

            case PKT_REGISTER:
                /* Klient podaje swój nick - zapamiętaj i potwierdź */
                snprintf(clients[i].name, sizeof(clients[i].name),
                         "%s", pkt.sender);
                printf("[serwer] Zarejestrowano '%s' (slot %d)\n",
                       clients[i].name, i);
                break;

            case PKT_LIST:
                handle_list(i);
                break;

            case PKT_2ALL:
                handle_2all(i, &pkt);
                break;

            case PKT_2ONE:
                handle_2one(i, &pkt);
                break;

            case PKT_STOP:
                /* Klient sam się wyrejestrował */
                printf("[serwer] STOP od '%s'\n", clients[i].name);
                remove_client(i);
                break;

            case PKT_ALIVE:
                /* Odpowiedź na ping - klient żyje */
                clients[i].alive_pending = 0;
                break;

            default:
                printf("[serwer] Nieznany typ pakietu %d od '%s'\n",
                       pkt.type, clients[i].name);
            }
        }
    }

    cleanup();
    return 0;
}
