#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include "common.h"

// Główna flaga sterująca pętlami wątków
volatile int running = 1;

// Kolejki FIFO dla klatek lewej i prawej kamery
frame_buffer_t left_buffer;
frame_buffer_t right_buffer;

// Zmienna współdzielona dla zsynchronizowanej pary stereo (synchronizacja mutex + cond_t)
stereo_pair_t global_stereo_pair;
int has_new_stereo = 0;
pthread_mutex_t mutex_stereo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_stereo = PTHREAD_COND_INITIALIZER;

// Zmienna współdzielona dla najnowszego stanu robota
robot_state_t global_robot_state;
int has_new_robot = 0;
pthread_mutex_t mutex_robot = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_robot = PTHREAD_COND_INITIALIZER;

// Sekcja liczników diagnostycznych
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
int total_left_frames = 0;
int total_right_frames = 0;
int total_stereo_pairs = 0;
int total_robot_states = 0;

// Handler sygnału SIGINT (przechwycenie CTRL+C)
void sigint_handler(int sig) {
    (void)sig;
    printf("\n[MAIN] Odebrano SIGINT. Zamykanie wątków...\n");
    running = 0;
    
    // Budzimy wątki uśpione na zmiennych warunkowych buforów (producent-konsument)
    pthread_cond_broadcast(&left_buffer.cond_not_empty);
    pthread_cond_broadcast(&left_buffer.cond_not_full);
    pthread_cond_broadcast(&right_buffer.cond_not_empty);
    pthread_cond_broadcast(&right_buffer.cond_not_full);
    
    // Budzimy wątek zapisu i wątek loggera uśpione w oczekiwaniu na dane
    pthread_cond_broadcast(&cond_stereo);
    pthread_cond_broadcast(&cond_robot);
}

// Lewa kamera (25 Hz)
void* left_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        push_frame(&left_buffer, frame);
        
        if (!running) break;
        
        pthread_mutex_lock(&stats_mutex);
        total_left_frames++;
        pthread_mutex_unlock(&stats_mutex);
        
        struct timespec delay = { 0, (38 + rand() % 5) * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[LEWA KAMERA] Wątek zakończony.\n");
    return NULL;
}

// Prawa kamera (25 Hz)
void* right_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        push_frame(&right_buffer, frame);
        
        if (!running) break;
        
        pthread_mutex_lock(&stats_mutex);
        total_right_frames++;
        pthread_mutex_unlock(&stats_mutex);
        
        struct timespec delay = { 0, (38 + rand() % 5) * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[PRAWA KAMERA] Wątek zakończony.\n");
    return NULL;
}

// Wątek synchronizacji klatek przy użyciu kolejek FIFO
// Implementacja sprytnego parowania: odrzucamy tylko przestarzałą klatkę
// (starszą o ponad 20 ms), a nowszą klatkę zachowujemy do porównania z kolejną z drugiego bufora.
void* sync_thread_func(void* arg) {
    (void)arg;
    camera_frame_t left;
    camera_frame_t right;
    int has_left = 0, has_right = 0;
    
    while (running) {
        if (!has_left) {
            left = pop_frame(&left_buffer);
            if (!running) break;
            has_left = 1;
        }
        if (!has_right) {
            right = pop_frame(&right_buffer);
            if (!running) break;
            has_right = 1;
        }
        
        double diff = timespec_diff_ms(left.timestamp, right.timestamp);
        
        if (fabs(diff) < 20.0) {
            // Pasują -> przesyłamy parę i sygnalizujemy cond_stereo
            stereo_pair_t pair = {left, right};
            
            pthread_mutex_lock(&mutex_stereo);
            global_stereo_pair = pair;
            has_new_stereo = 1;
            pthread_cond_signal(&cond_stereo);
            pthread_mutex_unlock(&mutex_stereo);
            
            pthread_mutex_lock(&stats_mutex);
            total_stereo_pairs++;
            pthread_mutex_unlock(&stats_mutex);
            
            printf("[SYNC] Sparowano (FIFO) L:%d, P:%d | Różnica: %.2f ms\n", 
                   left.frame_num, right.frame_num, diff);
            
            has_left = 0;
            has_right = 0;
        } else if (diff < -20.0) {
            // Klatka lewa jest za stara. Odrzucamy ją, a prawą zachowujemy.
            printf("[SYNC] Odrzucono lewą klatkę L:%d (Różnica: %.2f ms)\n", left.frame_num, diff);
            has_left = 0; 
        } else {
            // Klatka prawa jest za stara. Odrzucamy ją, a lewą zachowujemy.
            printf("[SYNC] Odrzucono prawą klatkę P:%d (Różnica: %.2f ms)\n", right.frame_num, diff);
            has_right = 0; 
        }
    }
    printf("[SYNC] Wątek zakończony.\n");
    return NULL;
}

// Wątek zapisu klatek na dysk (działa w tempie 10 Hz)
void* writer_thread_func(void* arg) {
    (void)arg;
    while (running) {
        pthread_mutex_lock(&mutex_stereo);
        // Oczekiwanie na nową parę zasygnalizowaną zmienną warunkową
        while (!has_new_stereo && running) {
            pthread_cond_wait(&cond_stereo, &mutex_stereo);
        }
        if (!running && !has_new_stereo) {
            pthread_mutex_unlock(&mutex_stereo);
            break;
        }
        stereo_pair_t pair = global_stereo_pair;
        has_new_stereo = 0;
        pthread_mutex_unlock(&mutex_stereo);
        
        char filename_l[64], filename_r[64];
        sprintf(filename_l, "left_%04d.jpg", pair.left.frame_num);
        sprintf(filename_r, "right_%04d.jpg", pair.right.frame_num);
        
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
        
        printf("[WRITER] Zapisano parę stereo (L:%d, P:%d)\n", pair.left.frame_num, pair.right.frame_num);
        
        struct timespec delay = { 0, 100 * 1000000L }; // 100 ms (10 Hz)
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[WRITER] Wątek zakończony.\n");
    return NULL;
}

// Wątek symulujący stan robota (100 Hz / co 10 ms)
void* robot_state_thread_func(void* arg) {
    (void)arg;
    double x = 0.0, y = 0.0, z = 0.0, yaw = 0.0;
    
    while (running) {
        x += 0.05 * cos(yaw);
        y += 0.05 * sin(yaw);
        yaw += 0.01;
        
        robot_state_t state = { x, y, z, 0.0, 0.0, yaw, get_current_time() };
        
        pthread_mutex_lock(&mutex_robot);
        global_robot_state = state;
        has_new_robot = 1;
        pthread_cond_signal(&cond_robot); // Budzenie loggera
        pthread_mutex_unlock(&mutex_robot);
        
        if (!running) break;
        
        pthread_mutex_lock(&stats_mutex);
        total_robot_states++;
        pthread_mutex_unlock(&stats_mutex);
        
        struct timespec delay = { 0, 10 * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[ROBOT STATE] Wątek zakończony.\n");
    return NULL;
}

// Logger stanu robota (działa okresowo 10 Hz)
void* logger_thread_func(void* arg) {
    (void)arg;
    FILE *log_file = fopen("robot_pose.log", "w");
    if (!log_file) return NULL;
    
    while (running) {
        pthread_mutex_lock(&mutex_robot);
        while (!has_new_robot && running) {
            pthread_cond_wait(&cond_robot, &mutex_robot);
        }
        if (!running && !has_new_robot) {
            pthread_mutex_unlock(&mutex_robot);
            break;
        }
        robot_state_t state = global_robot_state;
        has_new_robot = 0;
        pthread_mutex_unlock(&mutex_robot);
        
        fprintf(log_file, "[%ld.%09ld] Pozycja: (%.3f, %.3f, %.3f) Orientacja: (%.3f, %.3f, %.3f)\n",
                state.timestamp.tv_sec, state.timestamp.tv_nsec,
                state.x, state.y, state.z,
                state.roll, state.pitch, state.yaw);
        fflush(log_file);
        
        struct timespec delay = { 0, 100 * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    fclose(log_file);
    printf("[LOGGER] Wątek zakończony.\n");
    return NULL;
}

// Wątek diagnostyczny wyświetlający statystyki co 3 sekundy
void* stats_thread_func(void* arg) {
    (void)arg;
    struct timespec last_time = get_current_time();
    int last_l = 0, last_r = 0, last_s = 0, last_rob = 0;
    
    while (running) {
        sleep(3);
        if (!running) break;
        
        struct timespec now = get_current_time();
        double dt = timespec_diff_ms(last_time, now) / 1000.0;
        last_time = now;
        
        int l, r, s, rob;
        pthread_mutex_lock(&stats_mutex);
        l = total_left_frames;
        r = total_right_frames;
        s = total_stereo_pairs;
        rob = total_robot_states;
        pthread_mutex_unlock(&stats_mutex);
        
        printf("\n==================================================\n");
        printf("   DIAGNOSTYKA SYSTEMU (Okres pomiarowy: %.2fs)\n", dt);
        printf("--------------------------------------------------\n");
        printf("  Kamera LEWA  :  %d klatek (Średnia: %.2f Hz)\n", l, (l - last_l) / dt);
        printf("  Kamera PRAWA :  %d klatek (Średnia: %.2f Hz)\n", r, (r - last_r) / dt);
        printf("  Pary STEREO  :  %d par    (Średnia: %.2f Hz)\n", s, (s - last_s) / dt);
        printf("  Stan ROBOTA  :  %d próbek (Średnia: %.2f Hz)\n", rob, (rob - last_rob) / dt);
        printf("==================================================\n\n");
        
        last_l = l; last_r = r; last_s = s; last_rob = rob;
    }
    return NULL;
}

int main(void) {
    srand(time(NULL));
    printf("=== ROZPOCZYNAMY PROGRAM 2 (ZADANIE 2 - FIFO) ===\n");
    
    init_frame_buffer(&left_buffer);
    init_frame_buffer(&right_buffer);
    
    // Rejestracja obsługi sygnału SIGINT (CTRL+C)
    struct sigaction sa = { .sa_handler = sigint_handler };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    
    pthread_t left_cam_tid, right_cam_tid, sync_tid, writer_tid, robot_state_tid, logger_tid, stats_tid;
    
    pthread_create(&left_cam_tid, NULL, left_camera_thread_func, NULL);
    pthread_create(&right_cam_tid, NULL, right_camera_thread_func, NULL);
    pthread_create(&sync_tid, NULL, sync_thread_func, NULL);
    pthread_create(&writer_tid, NULL, writer_thread_func, NULL);
    pthread_create(&robot_state_tid, NULL, robot_state_thread_func, NULL);
    pthread_create(&logger_tid, NULL, logger_thread_func, NULL);
    pthread_create(&stats_tid, NULL, stats_thread_func, NULL);
    
    while (running) {
        sleep(1);
    }
    
    pthread_join(left_cam_tid, NULL);
    pthread_join(right_cam_tid, NULL);
    pthread_join(sync_tid, NULL);
    pthread_join(writer_tid, NULL);
    pthread_join(robot_state_tid, NULL);
    pthread_join(logger_tid, NULL);
    pthread_join(stats_tid, NULL);
    
    destroy_frame_buffer(&left_buffer);
    destroy_frame_buffer(&right_buffer);
    
    printf("=== PROGRAM 2 ZAKOŃCZONY ===\n");
    return 0;
}
