/*
 * chat.h - Wspólne definicje dla serwera i klienta
 *
 * Wszystko co musi być spójne po obu stronach leci tutaj -
 * nazwy kolejek, rozmiary, typy wiadomości i sama struktura.
 */

#ifndef CHAT_H
#define CHAT_H

/* Dobrze znana nazwa kolejki serwera - klienci muszą znać ten adres */
#define SERVER_QUEUE_NAME "/chat_server"

/* Maksymalna liczba jednoczesnych klientów */
#define MAX_CLIENTS 32

/* Maks. rozmiar treści wiadomości */
#define MAX_MSG_SIZE 512

/* Typy wiadomości (pole mtype w strukturze) */
#define MSG_INIT    1   /* klient się wita po raz pierwszy     */
#define MSG_CHAT    2   /* normalna wiadomość do broadcastu    */
#define MSG_ASSIGN  3   /* serwer odsyła przydzielony ID       */

/* Struktura wiadomości przesyłanej przez kolejkę POSIX */
typedef struct {
    long mtype;                 /* typ: INIT / CHAT / ASSIGN       */
    int  client_id;             /* ID klienta (0 przy INIT)        */
    char client_queue[64];      /* nazwa kolejki klienta            */
    char text[MAX_MSG_SIZE];    /* treść wiadomości                 */
} Message;

#endif /* CHAT_H */
