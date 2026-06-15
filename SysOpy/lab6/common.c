#include "common.h"
#include <stdio.h>
#include <stdlib.h>

// Pobranie czasu systemowego przy użyciu zegara monotonicznego
struct timespec get_current_time(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("[ERROR] clock_gettime nie powiodło się");
        exit(EXIT_FAILURE);
    }
    return ts;
}

// Obliczenie różnicy w milisekundach pomiędzy dwoma timespec (t2 - t1)
double timespec_diff_ms(struct timespec t1, struct timespec t2) {
    double s = (double)(t2.tv_sec - t1.tv_sec) * 1000.0;
    double ns = (double)(t2.tv_nsec - t1.tv_nsec) / 1000000.0;
    return s + ns;
}

// Inicjalizacja indeksów bufora oraz obiektów synchronizacji wątków
void init_frame_buffer(frame_buffer_t *buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->cond_not_empty, NULL);
    pthread_cond_init(&buf->cond_not_full, NULL);
}

// Niszczenie mutexu i zmiennych warunkowych
void destroy_frame_buffer(frame_buffer_t *buf) {
    pthread_mutex_destroy(&buf->mutex);
    pthread_cond_destroy(&buf->cond_not_empty);
    pthread_cond_destroy(&buf->cond_not_full);
}

// Wstawianie klatki na koniec kolejki (blokujące)
void push_frame(frame_buffer_t *buf, camera_frame_t frame) {
    pthread_mutex_lock(&buf->mutex);
    
    // Czekamy na sygnał, jeśli bufor jest pełny i program działa.
    // Używamy 'while', aby zapobiec problemom z fałszywymi wybudzeniami (spurious wakeups).
    while (buf->count == BUFFER_SIZE && running) {
        pthread_cond_wait(&buf->cond_not_full, &buf->mutex);
    }
    
    // Jeśli program kończy działanie, odblokowujemy mutex i wychodzimy
    if (!running) {
        pthread_mutex_unlock(&buf->mutex);
        return;
    }
    
    buf->data[buf->head] = frame;
    buf->head = (buf->head + 1) % BUFFER_SIZE; // Przesunięcie indeksu z zawijaniem
    buf->count++;
    
    // Sygnalizujemy wątkowi synchronizacji, że pojawiły się dane do pobrania
    pthread_cond_signal(&buf->cond_not_empty);
    pthread_mutex_unlock(&buf->mutex);
}

// Pobieranie klatki z początku kolejki (blokujące)
camera_frame_t pop_frame(frame_buffer_t *buf) {
    pthread_mutex_lock(&buf->mutex);
    
    // Czekamy, dopóki bufor jest pusty i program działa
    while (buf->count == 0 && running) {
        pthread_cond_wait(&buf->cond_not_empty, &buf->mutex);
    }
    
    // Jeśli zamykamy program i nie ma już danych, zwracamy zerową klatkę
    if (buf->count == 0 && !running) {
        pthread_mutex_unlock(&buf->mutex);
        camera_frame_t empty = {0};
        return empty;
    }
    
    camera_frame_t frame = buf->data[buf->tail];
    buf->tail = (buf->tail + 1) % BUFFER_SIZE; // Przesunięcie indeksu z zawijaniem
    buf->count--;
    
    // Sygnalizujemy wątkom kamer, że zwolniło się miejsce w buforze
    pthread_cond_signal(&buf->cond_not_full);
    pthread_mutex_unlock(&buf->mutex);
    return frame;
}
