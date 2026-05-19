/*
 * task3/main.c – Producer-Consumer + Manager (anti-starvation)
 *
 * Uruchomienie: ./main [N_prod] [M_cons] [K_buf]
 *
 * Rozszerzenie task2:
 *  - Dodatkowy proces Managera, który co 5 sekund:
 *      1. Drukuje raport stanu systemu
 *      2. Przenosi 1 zadanie z NORMAL -> PRIORITY (zapobiega starvation)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

#define TASK_LEN      11
#define MAX_BUF       64
#define SIM_TIME      20
#define PRIORITY_PROB 30
#define MGR_INTERVAL  5    /* manager odpala co tyle sekund */

#define SHM_NAME    "/t3_shm"
#define SEM_MTX     "/t3_mutex"
#define SEM_EMPTY_N "/t3_en"
#define SEM_FULL_N  "/t3_fn"
#define SEM_EMPTY_P "/t3_ep"
#define SEM_FULL_P  "/t3_fp"

typedef struct {
    char data[MAX_BUF][TASK_LEN];
    int  head, tail, count, capacity;
} Queue;

typedef struct {
    Queue        normal;
    Queue        priority;
    volatile int running;
    int          produced, consumed;
    int          promoted;   /* ile razy manager przeniósł zadanie */
} SharedMem;

static SharedMem *shm;
static sem_t *sem_mtx;
static sem_t *sem_en, *sem_fn;
static sem_t *sem_ep, *sem_fp;

static void open_sems(void) {
    sem_mtx = sem_open(SEM_MTX,     0);
    sem_en  = sem_open(SEM_EMPTY_N, 0);
    sem_fn  = sem_open(SEM_FULL_N,  0);
    sem_ep  = sem_open(SEM_EMPTY_P, 0);
    sem_fp  = sem_open(SEM_FULL_P,  0);
    if (sem_mtx == SEM_FAILED || sem_en == SEM_FAILED || sem_fn == SEM_FAILED ||
        sem_ep  == SEM_FAILED || sem_fp == SEM_FAILED) {
        perror("sem_open"); exit(1);
    }
}

static int twait(sem_t *s, int sek) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += sek;
    return sem_timedwait(s, &ts);
}

static void q_push(Queue *q, const char *t) {
    strncpy(q->data[q->tail], t, TASK_LEN - 1);
    q->data[q->tail][TASK_LEN - 1] = '\0';
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
}

static void q_pop(Queue *q, char *out) {
    strncpy(out, q->data[q->head], TASK_LEN);
    q->head = (q->head + 1) % q->capacity;
    q->count--;
}

/* ── PRODUCENT ── */
static void producer_loop(int id) {
    const char ch[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    int cs = (int)sizeof(ch) - 1;
    srand((unsigned)(time(NULL) ^ ((unsigned)getpid() << 8) ^ (unsigned)id));

    while (shm->running) {
        char task[TASK_LEN];
        for (int i = 0; i < 10; i++) task[i] = ch[rand() % cs];
        task[10] = '\0';

        int prio = (rand() % 100) < PRIORITY_PROB;

        if (prio) {
            if (twait(sem_ep, 1) != 0) continue;
            if (!shm->running) { sem_post(sem_ep); break; }
            sem_wait(sem_mtx);
            q_push(&shm->priority, task);
            shm->produced++;
            printf("[P%d] PRIORITY <- \"%s\"\n", id, task);
            fflush(stdout);
            sem_post(sem_mtx);
            sem_post(sem_fp);
        } else {
            if (twait(sem_en, 1) != 0) continue;
            if (!shm->running) { sem_post(sem_en); break; }
            sem_wait(sem_mtx);
            q_push(&shm->normal, task);
            shm->produced++;
            printf("[P%d] NORMAL   <- \"%s\"\n", id, task);
            fflush(stdout);
            sem_post(sem_mtx);
            sem_post(sem_fn);
        }

        usleep(300000 + (unsigned)(rand() % 500000));
    }

    printf("[P%d] koniec\n", id);
    exit(0);
}

/* ── KONSUMENT ── */
static void consumer_loop(int id) {
    char task[TASK_LEN];

    while (shm->running) {
        int got_prio = 0;

        /* PRIORITY najpierw */
        if (sem_trywait(sem_fp) == 0) {
            sem_wait(sem_mtx);
            q_pop(&shm->priority, task);
            shm->consumed++;
            sem_post(sem_mtx);
            sem_post(sem_ep);
            got_prio = 1;
        }

        if (got_prio) {
            printf("[C%d] PRIORITY -> \"", id);
        } else {
            if (twait(sem_fn, 1) != 0) continue;
            if (!shm->running) { sem_post(sem_fn); break; }
            sem_wait(sem_mtx);
            q_pop(&shm->normal, task);
            shm->consumed++;
            sem_post(sem_mtx);
            sem_post(sem_en);
            printf("[C%d] NORMAL   -> \"", id);
        }

        fflush(stdout);
        for (int i = 0; task[i] && shm->running; i++) {
            printf("%c", task[i]); fflush(stdout);
            usleep(300000);
        }
        printf("\"\n");
    }

    printf("[C%d] koniec\n", id);
    exit(0);
}

/* ── MANAGER ── */
static void manager_loop(void) {
    while (shm->running) {
        /* śpij MGR_INTERVAL sekund, ale co sekundę sprawdzaj running */
        for (int i = 0; i < MGR_INTERVAL && shm->running; i++)
            sleep(1);
        if (!shm->running) break;

        /* raport stanu */
        printf("\n[MGR] ====== RAPORT ======\n");
        printf("[MGR] Wyprodukowano: %d | Skonsumowano: %d | Awansowano: %d\n",
               shm->produced, shm->consumed, shm->promoted);
        printf("[MGR] NORMAL:   %d/%d\n", shm->normal.count,   shm->normal.capacity);
        printf("[MGR] PRIORITY: %d/%d\n", shm->priority.count, shm->priority.capacity);

        /*
         * Anti-starvation: wyciągnij 1 zadanie z NORMAL i wstaw do PRIORITY.
         * sem_trywait = nieblokujące – jeśli NORMAL pusta, nie czekamy.
         */
        if (sem_trywait(sem_fn) == 0) {
            /* pobierz z NORMAL */
            sem_wait(sem_mtx);
            char task[TASK_LEN];
            q_pop(&shm->normal, task);
            sem_post(sem_mtx);
            sem_post(sem_en);   /* zwolniliśmy slot w NORMAL */

            /* wstaw do PRIORITY (timedwait – może być pełna) */
            if (twait(sem_ep, 1) == 0) {
                sem_wait(sem_mtx);
                q_push(&shm->priority, task);
                shm->promoted++;
                sem_post(sem_mtx);
                sem_post(sem_fp);
                printf("[MGR] Awansowano NORMAL->PRIORITY: \"%s\"\n", task);
            } else {
                /* PRIORITY pełna – oddaj zadanie z powrotem */
                if (twait(sem_en, 1) == 0) {
                    sem_wait(sem_mtx);
                    q_push(&shm->normal, task);
                    sem_post(sem_mtx);
                    sem_post(sem_fn);
                }
                printf("[MGR] PRIORITY pelna – zadanie wróciło do NORMAL.\n");
            }
        } else {
            printf("[MGR] NORMAL pusta – brak zadań do awansowania.\n");
        }

        printf("[MGR] ====================\n\n");
    }

    printf("[MGR] koniec\n");
    exit(0);
}

/* ── MAIN ── */
int main(int argc, char *argv[]) {
    int N = argc > 1 ? atoi(argv[1]) : 3;
    int M = argc > 2 ? atoi(argv[2]) : 2;
    int K = argc > 3 ? atoi(argv[3]) : 8;

    if (N < 1 || M < 1 || K < 2 || K > MAX_BUF) {
        fprintf(stderr, "Użycie: %s [N>=1] [M>=1] [2<=K<=%d]\n", argv[0], MAX_BUF);
        return 1;
    }

    printf("[main] N=%d, M=%d, K=%d, czas=%ds  (30%% PRIORITY, manager co %ds)\n",
           N, M, K, SIM_TIME, MGR_INTERVAL);

    shm_unlink(SHM_NAME);
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedMem));
    shm = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    memset(shm, 0, sizeof(SharedMem));
    shm->normal.capacity   = K;
    shm->priority.capacity = K;
    shm->running           = 1;

    sem_unlink(SEM_MTX); sem_unlink(SEM_EMPTY_N); sem_unlink(SEM_FULL_N);
    sem_unlink(SEM_EMPTY_P); sem_unlink(SEM_FULL_P);
    sem_mtx = sem_open(SEM_MTX,     O_CREAT, 0666, 1);
    sem_en  = sem_open(SEM_EMPTY_N, O_CREAT, 0666, K);
    sem_fn  = sem_open(SEM_FULL_N,  O_CREAT, 0666, 0);
    sem_ep  = sem_open(SEM_EMPTY_P, O_CREAT, 0666, K);
    sem_fp  = sem_open(SEM_FULL_P,  O_CREAT, 0666, 0);

    /* forkuj producentów */
    for (int i = 0; i < N; i++)
        if (fork() == 0) { open_sems(); producer_loop(i); }

    /* forkuj konsumentów */
    for (int i = 0; i < M; i++)
        if (fork() == 0) { open_sems(); consumer_loop(i); }

    /* forkuj managera – jeden proces */
    if (fork() == 0) { open_sems(); manager_loop(); }

    sleep(SIM_TIME);
    printf("\n[main] stop!\n");
    shm->running = 0;

    /* odblokuj wszystkie procesy czekające na semaforach */
    for (int i = 0; i < N + M + 4; i++) {
        sem_post(sem_fn); sem_post(sem_fp);
        sem_post(sem_en); sem_post(sem_ep);
    }

    while (wait(NULL) > 0);

    /* zapisz statystyki PRZED munmap */
    int total_produced = shm->produced;
    int total_consumed = shm->consumed;
    int total_promoted = shm->promoted;

    munmap(shm, sizeof(SharedMem)); shm_unlink(SHM_NAME);
    sem_close(sem_mtx); sem_unlink(SEM_MTX);
    sem_close(sem_en);  sem_unlink(SEM_EMPTY_N);
    sem_close(sem_fn);  sem_unlink(SEM_FULL_N);
    sem_close(sem_ep);  sem_unlink(SEM_EMPTY_P);
    sem_close(sem_fp);  sem_unlink(SEM_FULL_P);

    printf("[main] Koniec. Wyprodukowano: %d | Skonsumowano: %d | Awansowano: %d\n",
           total_produced, total_consumed, total_promoted);
    return 0;
}
