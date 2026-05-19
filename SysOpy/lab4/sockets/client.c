/*
 * client.c - Klient czatu (TCP, POSIX sockets)
 *
 * Uruchomienie: ./client <nick> <adres_serwera> <port>
 *
 * Architektura: fork() - dziecko czyta wiadomości przychodzące od serwera
 * i wypisuje je na ekran; rodzic czyta stdin, parsuje komendy i wysyła pakiety.
 */

/* Wymagane żeby POSIX-owe funkcje (kill, fork, ...) były widoczne przy -std=c11 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.h"

/* Globale widoczne dla handlera sygnału */
static int    server_fd = -1;
static pid_t  child_pid = -1;
static char   my_name[MAX_NAME_LEN];

/* Wyślij STOP i zamknij połączenie */
static void cleanup(void) {
    if (server_fd >= 0) {
        Packet stop;
        memset(&stop, 0, sizeof(stop));
        stop.type = PKT_STOP;
        snprintf(stop.sender, sizeof(stop.sender), "%s", my_name);
        send_packet(server_fd, &stop);  
        close(server_fd); /* odłącza deskryptor pliku - zamyka połączenie*/
        server_fd = -1;
    }
    /* Zabij proces odbierający jeśli istnieje */
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        child_pid = -1;
    }
}

static void sig_handler(int sig) {
    (void)sig;
    cleanup();
    exit(0);
}

/*
 * Pętla dziecka - blokuje się na recv_packet i wypisuje to co dostanie.
 * Reaguje też na PKT_ALIVE (ping od serwera) odsyłając ponga.
 */
static void receiver_loop(int fd) {
    Packet pkt;
    while (1) {
        if (recv_packet(fd, &pkt) < 0) {
            printf("\n[!] Połączenie z serwerem zerwane.\n");
            kill(getppid(), SIGTERM);   /* powiadom rodzica */
            break;
        }

        if (pkt.type == PKT_MSG) {
            /* Wiadomość tekstowa od serwera */
            printf("\n<%s> %s\n", pkt.sender, pkt.text);
            fflush(stdout);

        } else if (pkt.type == PKT_ALIVE) {
            /* Ping od serwera - odpowiadamy od razu */
            Packet pong;
            memset(&pong, 0, sizeof(pong));
            pong.type = PKT_ALIVE;
            snprintf(pong.sender, sizeof(pong.sender), "%s", my_name);
            send_packet(fd, &pong);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Użycie: %s <nick> <adres_serwera> <port>\n", argv[0]);
        return 1;
    }

    snprintf(my_name, sizeof(my_name), "%s", argv[1]);
    const char *srv_addr = argv[2];
    int port = atoi(argv[3]);

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    /* Tworzymy gniazdo TCP i łączymy się z serwerem */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, srv_addr, &saddr.sin_addr) <= 0) {
        fprintf(stderr, "Nieprawidłowy adres: %s\n", srv_addr);
        close(server_fd); return 1;
    }

    if (connect(server_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect"); close(server_fd); return 1;
    }
    printf("[klient] Połączono z %s:%d\n", srv_addr, port);

    /* Zarejestruj się u serwera wysyłając swój nick */
    Packet reg;
    memset(&reg, 0, sizeof(reg));
    reg.type = PKT_REGISTER;
    snprintf(reg.sender, sizeof(reg.sender), "%s", my_name);
    if (send_packet(server_fd, &reg) < 0) {
        perror("send_packet REGISTER"); close(server_fd); return 1;
    }
    printf("[klient] Zarejestrowano jako '%s'. Komendy: LIST, 2ALL <tekst>, 2ONE <id> <tekst>, STOP\n",
           my_name);

    /*
     * Fork: dziecko - odbieranie od serwera i drukowanie
     *       rodzic  - czytanie stdin i wysyłanie pakietów
     */
    child_pid = fork();
    if (child_pid < 0) { perror("fork"); cleanup(); return 1; }

    if (child_pid == 0) {
        receiver_loop(server_fd);
        exit(0);
    }

    /* === RODZIC: parsowanie komend ze stdin === */
    char line[512];
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;   /* EOF - wychodzimy */

        /* Usuń newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strlen(line) == 0) continue;

        Packet out;
        memset(&out, 0, sizeof(out));
        snprintf(out.sender, sizeof(out.sender), "%s", my_name);

        /* parsujemy komendę do enuma */
        enum { CMD_LIST, CMD_2ALL, CMD_2ONE, CMD_STOP, CMD_UNKNOWN } cmd;

        if      (strcmp(line, "LIST") == 0)     cmd = CMD_LIST;
        else if (strncmp(line, "2ALL ", 5) == 0) cmd = CMD_2ALL;
        else if (strncmp(line, "2ONE ", 5) == 0) cmd = CMD_2ONE;
        else if (strcmp(line, "STOP") == 0)     cmd = CMD_STOP;
        else                                     cmd = CMD_UNKNOWN;

        switch (cmd) {

        case CMD_LIST:
            /* Prośba o listę aktywnych klientów */
            out.type = PKT_LIST;
            send_packet(server_fd, &out);
            break;

        case CMD_2ALL:
            /* Broadcast: 2ALL <treść> */
            out.type = PKT_2ALL;
            snprintf(out.text, sizeof(out.text), "%.*s", (int)(sizeof(out.text) - 1), line + 5);
            send_packet(server_fd, &out);
            break;

        case CMD_2ONE: {
            /* Prywatna: 2ONE <id> <treść> */
            int target_id;
            char rest[MAX_TEXT_LEN];
            if (sscanf(line + 5, "%d %479[^\n]", &target_id, rest) == 2) {
                out.type      = PKT_2ONE;
                out.target_id = (uint16_t)target_id;
                snprintf(out.text, sizeof(out.text), "%s", rest);
                send_packet(server_fd, &out);
            } else {
                printf("[!] Składnia: 2ONE <id> <treść>\n");
            }
            break;
        }

        case CMD_STOP:
            /* Wyrejestruj i wyjdź */
            out.type = PKT_STOP;
            send_packet(server_fd, &out);
            goto done;   /* break wyszłoby tylko ze switch, nie z pętli while */

        default:
            printf("[!] Nieznana komenda. Dostępne: LIST, 2ALL, 2ONE, STOP\n");
        }
    }

    done:
    cleanup();
    return 0;
}
