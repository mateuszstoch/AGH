#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#include "common.h"

// Flaga sterująca działaniem wątków. 
// volatile zapobiega optymalizacjom kompilatora polegającym na trzymaniu wartości w rejestrach CPU.
volatile int running = 1;

// Zmienne współdzielone dla kamer (Zadanie 1 - brak buforów cyklicznych)
// Dane są nadpisywane w sekcjach krytycznych i sygnalizowane semaforami POSIX.
camera_frame_t global_left_frame;
pthread_mutex_t mutex_left = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_left_new; 

camera_frame_t global_right_frame;
pthread_mutex_t mutex_right = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_right_new; 

// Para stereo przekazywana z wątku synchronizacji do zapisu
stereo_pair_t global_stereo_pair;
int has_new_stereo = 0; // Flaga oznaczająca nową parę do zapisu
pthread_mutex_t mutex_stereo = PTHREAD_MUTEX_INITIALIZER;

// Stan robota (sensorów)
robot_state_t global_robot_state;
pthread_mutex_t mutex_robot = PTHREAD_MUTEX_INITIALIZER;

// Wątek Lewej Kamery (generowanie klatek co ok. 40 ms / ~25 Hz)
void* left_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    
    // Przesunięcie startu wątków kamer, aby nie startowały synchronicznie
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        
        pthread_mutex_lock(&mutex_left);
        global_left_frame = frame;
        pthread_mutex_unlock(&mutex_left);
        
        // Powiadomienie wątku synchronizacji o nowej klatce
        sem_post(&sem_left_new);
        
        // Odstęp ok. 40 ms (25 Hz) z lekkim szumem (38-42 ms)
        struct timespec delay = { 0, (38 + rand() % 5) * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[LEWA KAMERA] Wątek zakończył działanie.\n");
    return NULL;
}

// Wątek Prawej Kamery (analogiczny)
void* right_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        
        pthread_mutex_lock(&mutex_right);
        global_right_frame = frame;
        pthread_mutex_unlock(&mutex_right);
        
        sem_post(&sem_right_new);
        
        struct timespec delay = { 0, (38 + rand() % 5) * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[PRAWA KAMERA] Wątek zakończył działanie.\n");
    return NULL;
}

// Wątek synchronizacji (parowanie klatek)
void* sync_thread_func(void* arg) {
    (void)arg;
    
    while (running) {
        // Blokowanie wątku w oczekiwaniu na nowe klatki z obu kamer
        sem_wait(&sem_left_new);
        sem_wait(&sem_right_new);
        
        if (!running) break;
        
        camera_frame_t left, right;
        
        pthread_mutex_lock(&mutex_left);
        left = global_left_frame;
        pthread_mutex_unlock(&mutex_left);
        
        pthread_mutex_lock(&mutex_right);
        right = global_right_frame;
        pthread_mutex_unlock(&mutex_right);
        
        // Porównanie różnicy czasów (warunek parowania < 20 ms)
        double diff = fabs(timespec_diff_ms(left.timestamp, right.timestamp));
        if (diff < 20.0) {
            pthread_mutex_lock(&mutex_stereo);
            global_stereo_pair.left = left;
            global_stereo_pair.right = right;
            has_new_stereo = 1;
            pthread_mutex_unlock(&mutex_stereo);
            printf("[SYNC] Sparowano klatki L:%d, P:%d | Różnica: %.2f ms\n", 
                   left.frame_num, right.frame_num, diff);
        } else {
            // Odrzucenie niesparowanych klatek
            printf("[SYNC] [WARNING] Niesparowane klatki L:%d i P:%d | Różnica: %.2f ms. Odrzucam.\n",
                   left.frame_num, right.frame_num, diff);
        }
    }
    printf("[SYNC] Wątek zakończył działanie.\n");
    return NULL;
}

// Zapisywacz danych na dysk (działa okresowo z częstotliwością 10 Hz)
void* writer_thread_func(void* arg) {
    (void)arg;
    
    while (running) {
        struct timespec delay = { 0, 100 * 1000000L }; // 100 ms
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
        
        if (!running) break;
        
        int has_pair = 0;
        stereo_pair_t pair;
        
        pthread_mutex_lock(&mutex_stereo);
        if (has_new_stereo) {
            pair = global_stereo_pair;
            has_new_stereo = 0; // Reset flagi zapisu
            has_pair = 1;
        }
        pthread_mutex_unlock(&mutex_stereo);
        
        if (has_pair) {
            char filename_l[64], filename_r[64];
            sprintf(filename_l, "left_%04d.jpg", pair.left.frame_num);
            sprintf(filename_r, "right_%04d.jpg", pair.right.frame_num);
            
            // Zapis klatek do plików testowych (symulacja obrazów)
            FILE *f_l = fopen(filename_l, "w");
            if (f_l) {
                fprintf(f_l, "FRAME_NUM: %d\nTIMESTAMP: %ld.%09ld\n", 
                        pair.left.frame_num, pair.left.timestamp.tv_sec, pair.left.timestamp.tv_nsec);
                fclose(f_l);
            }
            FILE *f_r = fopen(filename_r, "w");
            if (f_r) {
                fprintf(f_r, "FRAME_NUM: %d\nTIMESTAMP: %ld.%09ld\n", 
                        pair.right.frame_num, pair.right.timestamp.tv_sec, pair.right.timestamp.tv_nsec);
                fclose(f_r);
            }
            printf("[WRITER] Zapisano klatki do plików: %s i %s\n", filename_l, filename_r);
        }
    }
    printf("[WRITER] Wątek zakończył działanie.\n");
    return NULL;
}

// Wątek generujący stan robota (częstotliwość 100 Hz / co 10 ms)
void* robot_state_thread_func(void* arg) {
    (void)arg;
    double x = 0.0, y = 0.0, z = 0.0, yaw = 0.0;
    
    while (running) {
        // Symulacja ruchu robota po okręgu
        x += 0.05 * cos(yaw);
        y += 0.05 * sin(yaw);
        yaw += 0.01;
        if (yaw > 2 * M_PI) yaw -= 2 * M_PI;
        
        robot_state_t state = { x, y, z, 0.0, 0.0, yaw, get_current_time() };
        
        pthread_mutex_lock(&mutex_robot);
        global_robot_state = state;
        pthread_mutex_unlock(&mutex_robot);
        
        struct timespec delay = { 0, 10 * 1000000L }; // 10 ms
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[ROBOT STATE] Wątek zakończył działanie.\n");
    return NULL;
}

// Logger stanu robota (działa okresowo z częstotliwością 10 Hz / co 100 ms)
void* logger_thread_func(void* arg) {
    (void)arg;
    FILE *log_file = fopen("robot_pose.log", "w");
    if (!log_file) {
        perror("[ERROR] Błąd otwarcia pliku logu");
        return NULL;
    }
    
    while (running) {
        struct timespec delay = { 0, 100 * 1000000L }; // 100 ms
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
        
        if (!running) break;
        
        robot_state_t state;
        pthread_mutex_lock(&mutex_robot);
        state = global_robot_state;
        pthread_mutex_unlock(&mutex_robot);
        
        fprintf(log_file, "[%ld.%09ld] Pozycja: (%.3f, %.3f, %.3f) Orientacja: (%.3f, %.3f, %.3f)\n",
                state.timestamp.tv_sec, state.timestamp.tv_nsec,
                state.x, state.y, state.z,
                state.roll, state.pitch, state.yaw);
        fflush(log_file); // Wymuszenie fizycznego zapisu na dysku
    }
    fclose(log_file);
    printf("[LOGGER] Wątek zakończył działanie i zapisał logi.\n");
    return NULL;
}

int main(void) {
    srand(time(NULL));
    printf("=== URUCHAMIAMY PROGRAM 1 (ZADANIE 1) ===\n");
    
    sem_init(&sem_left_new, 0, 0);
    sem_init(&sem_right_new, 0, 0);
    
    pthread_t left_cam_tid, right_cam_tid, sync_tid, writer_tid, robot_state_tid, logger_tid;
    
    // Uruchomienie wątków pobocznych
    pthread_create(&left_cam_tid, NULL, left_camera_thread_func, NULL);
    pthread_create(&right_cam_tid, NULL, right_camera_thread_func, NULL);
    pthread_create(&sync_tid, NULL, sync_thread_func, NULL);
    pthread_create(&writer_tid, NULL, writer_thread_func, NULL);
    pthread_create(&robot_state_tid, NULL, robot_state_thread_func, NULL);
    pthread_create(&logger_tid, NULL, logger_thread_func, NULL);
    
    printf("[MAIN] System działa. Czekamy 20 sekund...\n");
    sleep(20);
    
    printf("[MAIN] Czas minął. Zamykanie wątków...\n");
    running = 0;
    
    // Wybudzenie wątku synchronizacji z sem_wait w celu umożliwienia zakończenia pętli
    sem_post(&sem_left_new);
    sem_post(&sem_right_new);
    
    // Dołączanie wątków w celu zwolnienia zasobów
    pthread_join(left_cam_tid, NULL);
    pthread_join(right_cam_tid, NULL);
    pthread_join(sync_tid, NULL);
    pthread_join(writer_tid, NULL);
    pthread_join(robot_state_tid, NULL);
    pthread_join(logger_tid, NULL);
    
    sem_destroy(&sem_left_new);
    sem_destroy(&sem_right_new);
    
    printf("=== PROGRAM 1 ZAKOŃCZONY ===\n");
    return 0;
}
