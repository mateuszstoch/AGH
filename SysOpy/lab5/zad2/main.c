/*
 * task2/main.c – Producer-Consumer z dwiema kolejkami: NORMAL i PRIORITY
 *
 * Uruchomienie: ./main [N_prod] [M_cons] [K_buf]
 *
 * Rozszerzenie task1:
 *  - 30% zadań trafia do PRIORITY, 70% do NORMAL
 *  - Konsumenci zawsze sprawdzają PRIORITY najpierw (sem_trywait)
 *  - Osobna para semaforów empty/full dla każdej kolejki
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
#define PRIORITY_PROB 30   /* % zadań do PRIORITY */

#define SHM_NAME    "/t2_shm"
#define SEM_MTX     "/t2_mutex"
#define SEM_EMPTY_N "/t2_en"
#define SEM_FULL_N  "/t2_fn"
#define SEM_EMPTY_P "/t2_ep"
#define SEM_FULL_P  "/t2_fp"

/* kolejka cykliczna */
typedef struct {
    char data[MAX_BUF][TASK_LEN];
    int  head, tail, count, capacity;
} Queue;

typedef struct {
    Queue        normal;
    Queue        priority;
    volatile int running;
    int          produced, consumed;
} SharedMem;

static SharedMem *shm;
static sem_t *sem_mtx;
static sem_t *sem_en, *sem_fn;   /* empty/full NORMAL   */
static sem_t *sem_ep, *sem_fp;   /* empty/full PRIORITY */

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
            /* zapisz do kolejki PRIORITY */
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
            /* zapisz do kolejki NORMAL */
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

        /* najpierw spróbuj wziąć z PRIORITY (nieblokująco) */
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
            /* brak PRIORITY – czekaj na NORMAL */
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

/* ── MAIN ── */
int main(int argc, char *argv[]) {
    int N = argc > 1 ? atoi(argv[1]) : 3;
    int M = argc > 2 ? atoi(argv[2]) : 2;
    int K = argc > 3 ? atoi(argv[3]) : 8;

    if (N < 1 || M < 1 || K < 2 || K > MAX_BUF) {
        fprintf(stderr, "Użycie: %s [N>=1] [M>=1] [2<=K<=%d]\n", argv[0], MAX_BUF);
        return 1;
    }

    printf("[main] N=%d, M=%d, K=%d, czas=%ds  (30%% trafia do PRIORITY)\n",
           N, M, K, SIM_TIME);

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

    for (int i = 0; i < N; i++)
        if (fork() == 0) { open_sems(); producer_loop(i); }
    for (int i = 0; i < M; i++)
        if (fork() == 0) { open_sems(); consumer_loop(i); }

    sleep(SIM_TIME);
    printf("\n[main] stop!\n");
    shm->running = 0;
    for (int i = 0; i < N + M + 2; i++) {
        sem_post(sem_fn); sem_post(sem_fp);
        sem_post(sem_en); sem_post(sem_ep);
    }

    while (wait(NULL) > 0);

    /* zapisz statystyki PRZED munmap */
    int total_produced = shm->produced;
    int total_consumed = shm->consumed;

    munmap(shm, sizeof(SharedMem)); shm_unlink(SHM_NAME);
    sem_close(sem_mtx); sem_unlink(SEM_MTX);
    sem_close(sem_en);  sem_unlink(SEM_EMPTY_N);
    sem_close(sem_fn);  sem_unlink(SEM_FULL_N);
    sem_close(sem_ep);  sem_unlink(SEM_EMPTY_P);
    sem_close(sem_fp);  sem_unlink(SEM_FULL_P);

    printf("[main] Wyprodukowano: %d | Skonsumowano: %d\n",
           total_produced, total_consumed);
    return 0;
}
