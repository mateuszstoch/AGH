#ifndef CHAT_H
#define CHAT_H

#include <stdint.h>

/* Maksymalna liczba jednoczesnych klientów */
#define MAX_CLIENTS   32

/* Maks. długość nazwy/nicku klienta */
#define MAX_NAME_LEN  32

/* Maks. długość treści wiadomości */
#define MAX_TEXT_LEN  480

/* Typy pakietów - jednoznacznie określają co serwer/klient ma zrobić */
#define PKT_REGISTER  1   /* klient podaje swój nick przy rejestracji   */
#define PKT_LIST      2   /* klient żąda listy aktywnych klientów        */
#define PKT_2ALL      3   /* broadcast do wszystkich poza nadawcą        */
#define PKT_2ONE      4   /* wiadomość do konkretnego klienta (po ID)    */
#define PKT_STOP      5   /* klient informuje o zamknięciu               */
#define PKT_ALIVE     6   /* ping serwera / pong klienta                 */
#define PKT_MSG       7   /* gotowa wiadomość od serwera do klienta      */


typedef struct {
    uint8_t  type;                  /* jeden z PKT_* powyżej              */
    uint16_t target_id;             /* ID adresata (tylko dla PKT_2ONE)   */
    char     sender[MAX_NAME_LEN];  /* nick nadawcy / klienta             */
    char     text[MAX_TEXT_LEN];    /* treść, lista, data+wiad. itp.      */
} __attribute__((packed)) Packet;

/*
 * Pomocnik do wysyłania całego pakietu przez socket -
 * send() może zapisać mniej bajtów niż prosimy, więc używamy pętli.
 */
#include <sys/types.h>
#include <sys/socket.h>

static inline int send_packet(int fd, const Packet *p) {
    const char *buf = (const char *)p;
    size_t total = sizeof(Packet);
    size_t sent  = 0;
    while (sent < total) {
        ssize_t n = send(fd, buf + sent, total - sent, MSG_NOSIGNAL);
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

/*
 * Pomocnik do odbierania całego pakietu -
 * recv() też może dać mniej bajtów, więc używamy pętli.
 * Zwraca 0 gdy OK, -1 gdy błąd lub połączenie zamknięte.
 */
static inline int recv_packet(int fd, Packet *p) {
    char *buf    = (char *)p;
    size_t total = sizeof(Packet);
    size_t got   = 0;
    while (got < total) {
        ssize_t n = recv(fd, buf + got, total - got, 0);
        if (n <= 0) return -1;
        got += (size_t)n;
    }
    return 0;
}

#endif /* CHAT_H */
