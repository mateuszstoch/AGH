#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdatomic.h>
#include <sched.h>
#include "common.h"

volatile int running = 1;

// Kolejki FIFO dla kamer
frame_buffer_t left_buffer;
frame_buffer_t right_buffer;

// Przekazywanie pary stereo (synchronizacja mutex + cond_t)
stereo_pair_t global_stereo_pair;
int has_new_stereo = 0;
pthread_mutex_t mutex_stereo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_stereo = PTHREAD_COND_INITIALIZER;

// Przekazywanie stanu robota
robot_state_t global_robot_state;
int has_new_robot = 0;
pthread_mutex_t mutex_robot = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_robot = PTHREAD_COND_INITIALIZER;

// Zmienne atomowe (stdatomic.h) do bezpiecznego zliczania ramek i ostrzeżeń.
// Inkrementacja atomic_int odbywa się bez użycia mutexów (niepodzielne instrukcje CPU).
atomic_int total_left_frames = ATOMIC_VAR_INIT(0);
atomic_int total_right_frames = ATOMIC_VAR_INIT(0);
atomic_int total_stereo_pairs = ATOMIC_VAR_INIT(0);
atomic_int total_robot_states = ATOMIC_VAR_INIT(0);
atomic_int watchdog_warnings = ATOMIC_VAR_INIT(0);

// Zmienne czasowe do monitorowania opóźnień przez wątek Watchdoga
pthread_mutex_t watchdog_mutex = PTHREAD_MUTEX_INITIALIZER;
struct timespec last_left_time;
struct timespec last_right_time;
struct timespec last_robot_time;

// Funkcja pomocnicza do ustawiania polityki czasu rzeczywistego w Linuxie.
// Używamy polityki SCHED_FIFO, która wywłaszcza standardowe wątki (SCHED_OTHER).
// Uwaga: Wymaga uprawnień administratora (sudo).
void set_thread_priority(pthread_t thread, int policy, int priority_val) {
    struct sched_param param = { .sched_priority = priority_val };
    int ret = pthread_setschedparam(thread, policy, &param);
    if (ret != 0) {
        fprintf(stderr, "[WARNING] Nie można ustawić priorytetu RT dla wątku (brak uprawnień root/sudo).\n");
    } else {
        printf("[RT] Wątek ustawiony na %s z priorytetem %d.\n",
               (policy == SCHED_FIFO) ? "SCHED_FIFO" : "SCHED_RR", priority_val);
    }
}

// Handler CTRL+C
void sigint_handler(int sig) {
    (void)sig;
    printf("\n[MAIN] Odebrano SIGINT. Zamykanie wątków czasu rzeczywistego...\n");
    running = 0;
    
    pthread_cond_broadcast(&left_buffer.cond_not_empty);
    pthread_cond_broadcast(&left_buffer.cond_not_full);
    pthread_cond_broadcast(&right_buffer.cond_not_empty);
    pthread_cond_broadcast(&right_buffer.cond_not_full);
    
    pthread_cond_broadcast(&cond_stereo);
    pthread_cond_broadcast(&cond_robot);
}

// Lewa kamera (symuluje opóźnienie sprzętowe co 100 klatek, by wywołać reakcję Watchdoga)
void* left_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        
        pthread_mutex_lock(&watchdog_mutex);
        last_left_time = frame.timestamp;
        pthread_mutex_unlock(&watchdog_mutex);
        
        push_frame(&left_buffer, frame);
        if (!running) break;
        
        atomic_fetch_add(&total_left_frames, 1);
        
        struct timespec delay = { 0, 0 };
        if (frame_counter % 100 == 0) {
            printf("[LEWA KAMERA] (Symulacja opóźnienia kamer - 120 ms...)\n");
            delay.tv_nsec = 120 * 1000000L;
        } else {
            delay.tv_nsec = (38 + rand() % 5) * 1000000L;
        }
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[LEWA KAMERA] Wątek zakończony.\n");
    return NULL;
}

// Prawa kamera (symuluje opóźnienie co 130 klatek)
void* right_camera_thread_func(void* arg) {
    (void)arg;
    int frame_counter = 0;
    usleep((rand() % 20) * 1000);
    
    while (running) {
        camera_frame_t frame = { ++frame_counter, get_current_time() };
        
        pthread_mutex_lock(&watchdog_mutex);
        last_right_time = frame.timestamp;
        pthread_mutex_unlock(&watchdog_mutex);
        
        push_frame(&right_buffer, frame);
        if (!running) break;
        
        atomic_fetch_add(&total_right_frames, 1);
        
        struct timespec delay = { 0, 0 };
        if (frame_counter % 130 == 0) {
            printf("[PRAWA KAMERA] (Symulacja opóźnienia kamer - 100 ms...)\n");
            delay.tv_nsec = 100 * 1000000L;
        } else {
            delay.tv_nsec = (38 + rand() % 5) * 1000000L;
        }
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[PRAWA KAMERA] Wątek zakończony.\n");
    return NULL;
}

// Synchronizacja klatek z buforów FIFO kamer
void* sync_thread_func(void* arg) {
    (void)arg;
    camera_frame_t left, right;
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
            stereo_pair_t pair = {left, right};
            
            pthread_mutex_lock(&mutex_stereo);
            global_stereo_pair = pair;
            has_new_stereo = 1;
            pthread_cond_signal(&cond_stereo);
            pthread_mutex_unlock(&mutex_stereo);
            
            atomic_fetch_add(&total_stereo_pairs, 1);
            
            printf("[SYNC] Sparowano L:%d, P:%d | Różnica: %.2f ms\n", left.frame_num, right.frame_num, diff);
            has_left = 0; has_right = 0;
        } else if (diff < -20.0) {
            printf("[SYNC] Odrzucono lewą klatkę L:%d (Różnica: %.2f ms)\n", left.frame_num, diff);
            has_left = 0;
        } else {
            printf("[SYNC] Odrzucono prawą klatkę P:%d (Różnica: %.2f ms)\n", right.frame_num, diff);
            has_right = 0;
        }
    }
    printf("[SYNC] Wątek zakończony.\n");
    return NULL;
}

// Zapis pary stereo do plików jpg (10 Hz)
void* writer_thread_func(void* arg) {
    (void)arg;
    while (running) {
        pthread_mutex_lock(&mutex_stereo);
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
        
        struct timespec delay = { 0, 100 * 1000000L };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[WRITER] Wątek zakończony.\n");
    return NULL;
}

// Stan robota (100 Hz / co 10 ms)
// Symuluje również opóźnienie obliczeniowe co 300 klatek, by wyzwolić reakcję Watchdoga.
void* robot_state_thread_func(void* arg) {
    (void)arg;
    double x = 0.0, y = 0.0, z = 0.0, yaw = 0.0;
    int state_counter = 0;
    
    while (running) {
        state_counter++;
        x += 0.05 * cos(yaw);
        y += 0.05 * sin(yaw);
        yaw += 0.01;
        
        robot_state_t state = { x, y, z, 0.0, 0.0, yaw, get_current_time() };
        
        pthread_mutex_lock(&watchdog_mutex);
        last_robot_time = state.timestamp;
        pthread_mutex_unlock(&watchdog_mutex);
        
        pthread_mutex_lock(&mutex_robot);
        global_robot_state = state;
        has_new_robot = 1;
        pthread_cond_signal(&cond_robot);
        pthread_mutex_unlock(&mutex_robot);
        
        if (!running) break;
        
        atomic_fetch_add(&total_robot_states, 1);
        
        struct timespec delay = { 0, 0 };
        if (state_counter % 300 == 0) {
            printf("[ROBOT STATE] (Symulacja laga obliczeniowego - 40 ms...)\n");
            delay.tv_nsec = 40 * 1000000L;
        } else {
            delay.tv_nsec = 10 * 1000000L;
        }
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
    }
    printf("[ROBOT STATE] Wątek zakończony.\n");
    return NULL;
}

// Logger stanu robota (10 Hz)
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

// Wątek Watchdoga (Najwyższy priorytet czasu rzeczywistego)
// Wykrywa przekroczenia czasu (deadlinów): 80ms dla kamer, 25ms dla pomiarów stanu robota.
void* watchdog_thread_func(void* arg) {
    (void)arg;
    while (running) {
        struct timespec delay = { 0, 20 * 1000000L }; // Sprawdzanie co 20 ms
        clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
        if (!running) break;
        
        struct timespec now = get_current_time();
        struct timespec l_time, r_time, rob_time;
        
        pthread_mutex_lock(&watchdog_mutex);
        l_time = last_left_time; r_time = last_right_time; rob_time = last_robot_time;
        pthread_mutex_unlock(&watchdog_mutex);
        
        double dt_l = timespec_diff_ms(l_time, now);
        double dt_r = timespec_diff_ms(r_time, now);
        double dt_rob = timespec_diff_ms(rob_time, now);
        
        // Lewa kamera deadline: 80 ms (2x okres pracy 25Hz)
        if (dt_l > 80.0) {
            printf("[WATCHDOG] [WARNING] LEFT CAMERA SLOW! Brak klatki od %.1f ms!\n", dt_l);
            atomic_fetch_add(&watchdog_warnings, 1);
        }
        // Prawa kamera deadline: 80 ms
        if (dt_r > 80.0) {
            printf("[WATCHDOG] [WARNING] RIGHT CAMERA SLOW! Brak klatki od %.1f ms!\n", dt_r);
            atomic_fetch_add(&watchdog_warnings, 1);
        }
        // Stan robota deadline: 25 ms (2.5x okres pracy 100Hz)
        if (dt_rob > 25.0) {
            printf("[WATCHDOG] [WARNING] ROBOT STATE SLOW! Brak stanu od %.1f ms!\n", dt_rob);
            atomic_fetch_add(&watchdog_warnings, 1);
        }
    }
    printf("[WATCHDOG] Wątek zakończony.\n");
    return NULL;
}

// Generowanie pliku raportu końcowego
void generate_final_report(double total_time_s) {
    FILE *rep = fopen("report.txt", "w");
    if (!rep) return;
    
    // Pobranie wartości zmiennych atomowych
    int l = atomic_load(&total_left_frames);
    int r = atomic_load(&total_right_frames);
    int s = atomic_load(&total_stereo_pairs);
    int rob = atomic_load(&total_robot_states);
    int warnings = atomic_load(&watchdog_warnings);
    
    fprintf(rep, "==================================================\n");
    fprintf(rep, "       RAPORT KOŃCOWY DZIAŁANIA SYSTEMU (RT)      \n");
    fprintf(rep, "==================================================\n");
    fprintf(rep, " Czas trwania testu:   %.2f sekund\n", total_time_s);
    fprintf(rep, " Licznik klatek:\n");
    fprintf(rep, "   - Lewa kamera:      %d (Średnia częstotliwość: %.2f Hz)\n", l, l / total_time_s);
    fprintf(rep, "   - Prawa kamera:     %d (Średnia częstotliwość: %.2f Hz)\n", r, r / total_time_s);
    fprintf(rep, "   - Pary stereo:      %d (Średnia częstotliwość: %.2f Hz)\n", s, s / total_time_s);
    fprintf(rep, " Odczyty stanu robota: %d (Średnia częstotliwość: %.2f Hz)\n", rob, rob / total_time_s);
    fprintf(rep, " Liczba alarmów WD:    %d\n", warnings);
    fprintf(rep, "==================================================\n");
    
    fclose(rep);
    printf("[MAIN] Raport końcowy zapisany w pliku 'report.txt'.\n");
}

int main(void) {
    srand(time(NULL));
    printf("=== ROZPOCZYNAMY PROGRAM 3 (ZADANIE 3 - REAL-TIME) ===\n");
    
    init_frame_buffer(&left_buffer);
    init_frame_buffer(&right_buffer);
    
    struct timespec start_time = get_current_time();
    last_left_time = start_time; last_right_time = start_time; last_robot_time = start_time;
    
    struct sigaction sa = { .sa_handler = sigint_handler };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    
    pthread_t left_cam_tid, right_cam_tid, sync_tid, writer_tid, robot_state_tid, logger_tid, watchdog_tid;
    
    pthread_create(&left_cam_tid, NULL, left_camera_thread_func, NULL);
    pthread_create(&right_cam_tid, NULL, right_camera_thread_func, NULL);
    pthread_create(&sync_tid, NULL, sync_thread_func, NULL);
    pthread_create(&writer_tid, NULL, writer_thread_func, NULL);
    pthread_create(&robot_state_tid, NULL, robot_state_thread_func, NULL);
    pthread_create(&logger_tid, NULL, logger_thread_func, NULL);
    pthread_create(&watchdog_tid, NULL, watchdog_thread_func, NULL);
    
    // Ustawienie priorytetów czasu rzeczywistego (SCHED_FIFO)
    // Wątek Watchdoga (priorytet 90) oraz wątek stanu robota (priorytet 80)
    printf("[MAIN] Konfiguracja harmonogramu czasu rzeczywistego (SCHED_FIFO)...\n");
    set_thread_priority(watchdog_tid, SCHED_FIFO, 90);
    set_thread_priority(robot_state_tid, SCHED_FIFO, 80);
    
    printf("[MAIN] System uruchomiony. Działanie przez 20s (lub do naciśnięcia CTRL+C).\n");
    
    int elapsed = 0;
    while (running && elapsed < 20) {
        sleep(1);
        elapsed++;
    }
    
    running = 0;
    
    // Dołączanie wątków
    pthread_join(left_cam_tid, NULL);
    pthread_join(right_cam_tid, NULL);
    pthread_join(sync_tid, NULL);
    pthread_join(writer_tid, NULL);
    pthread_join(robot_state_tid, NULL);
    pthread_join(logger_tid, NULL);
    pthread_join(watchdog_tid, NULL);
    
    struct timespec end_time = get_current_time();
    double total_time_s = timespec_diff_ms(start_time, end_time) / 1000.0;
    
    generate_final_report(total_time_s);
    
    destroy_frame_buffer(&left_buffer);
    destroy_frame_buffer(&right_buffer);
    
    printf("=== PROGRAM 3 ZAKOŃCZONY ===\n");
    return 0;
}
