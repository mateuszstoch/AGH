#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 10

// Globalna flaga sterująca pętlami w wątkach
extern volatile int running;

// Struktura pojedynczej klatki z kamery
typedef struct {
    int frame_num;               // Numer kolejny klatki
    struct timespec timestamp;   // Dokładny czas wygenerowania klatki
} camera_frame_t;

// Struktura przechowująca parę klatek (stereo)
typedef struct {
    camera_frame_t left;
    camera_frame_t right;
} stereo_pair_t;

// Struktura reprezentująca stan robota (odczyty z sensorów)
typedef struct {
    double x, y, z;              // Pozycja w przestrzeni 3D
    double roll, pitch, yaw;     // Kąty orientacji robota
    struct timespec timestamp;   // Czas pomiaru stanu
} robot_state_t;

// Bufor cykliczny (kolejka FIFO) dla klatek z kamer.
// Head, tail i count służą do bezkopijkowej obsługi kolejki (modulo).
// Mutex i zmienne warunkowe realizują synchronizację producent-konsument.
typedef struct {
    camera_frame_t data[BUFFER_SIZE];
    int head;                    // Indeks zapisu (producent)
    int tail;                    // Indeks odczytu (konsument)
    int count;                   // Bieżąca liczba elementów w buforze
    pthread_mutex_t mutex;       // Mutex chroniący sekcję krytyczną bufora
    pthread_cond_t cond_not_empty; // Sygnalizacja, że w buforze są dane do odczytu
    pthread_cond_t cond_not_full;  // Sygnalizacja, że w buforze zwolniło się miejsce
} frame_buffer_t;

// Funkcje pomocnicze czasu
struct timespec get_current_time(void);
double timespec_diff_ms(struct timespec t1, struct timespec t2);

// Funkcje obsługi bufora cyklicznego kamer
void init_frame_buffer(frame_buffer_t *buf);
void destroy_frame_buffer(frame_buffer_t *buf);
void push_frame(frame_buffer_t *buf, camera_frame_t frame);
camera_frame_t pop_frame(frame_buffer_t *buf);

#endif // COMMON_H
