/*
 * task1/main.c – Prosty producer-consumer z POSIX IPC
 *
 * Uruchomienie: ./main [N_prod] [M_cons] [K_buf]
 * Domyślnie:    N=3, M=2, K=8, symulacja=20s
 *
 * Synchronizacja klasyczna: semafor empty, full, mutex
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

#define TASK_LEN  11    /* 10 znaków + null terminator */
#define MAX_BUF   64    /* max K */
#define SIM_TIME  20    /* czas symulacji w sekundach  */

/* nazwy obiektów IPC */
#define SHM_NAME  "/t1_shm"
#define SEM_MTX   "/t1_mutex"
#define SEM_EMPTY "/t1_empty"
#define SEM_FULL  "/t1_full"

/* bufor cykliczny w shared memory */
typedef struct {
    char data[MAX_BUF][TASK_LEN];
    int  head, tail, count;
    int  capacity;
    volatile int running;
    int  produced, consumed;
} SharedMem;

static SharedMem *shm;
static sem_t *sem_mtx, *sem_empty, *sem_full;

/* otwiera semafory po fork() – każde dziecko musi to wywołać */
static void open_sems(void) {
    sem_mtx   = sem_open(SEM_MTX,   0);
    sem_empty = sem_open(SEM_EMPTY, 0);
    sem_full  = sem_open(SEM_FULL,  0);
    if (sem_mtx == SEM_FAILED || sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        perror("sem_open"); exit(1);
    }
}

/* sem_timedwait z timeoutem 'sek' sekund od teraz */
static int twait(sem_t *s, int sek) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += sek;
    return sem_timedwait(s, &ts);
}

/* ── PRODUCENT ── */
static void producer_loop(int id) {
    const char ch[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    int cs = (int)sizeof(ch) - 1;
    srand((unsigned)(time(NULL) ^ ((unsigned)getpid() << 8)));

    while (shm->running) {
        /* generuj losowy string 10 znaków */
        char task[TASK_LEN];
        for (int i = 0; i < 10; i++) task[i] = ch[rand() % cs];
        task[10] = '\0';

        /* czekaj na wolne miejsce (timedwait żeby reagować na running=0) */
        if (twait(sem_empty, 1) != 0) continue;
        if (!shm->running) { sem_post(sem_empty); break; }

        sem_wait(sem_mtx);
        /* wstaw do bufora */
        strncpy(shm->data[shm->tail], task, TASK_LEN);
        shm->tail = (shm->tail + 1) % shm->capacity;
        shm->count++;
        shm->produced++;
        printf("[P%d] dodano: \"%s\"  (bufor: %d/%d)\n",
               id, task, shm->count, shm->capacity);
        fflush(stdout);
        sem_post(sem_mtx);
        sem_post(sem_full);  /* jest nowe zadanie */

        usleep(300000 + (unsigned)(rand() % 500000));
    }

    printf("[P%d] koniec\n", id);
    exit(0);
}

/* ── KONSUMENT ── */
static void consumer_loop(int id) {
    char task[TASK_LEN];

    while (shm->running) {
        /* czekaj aż będzie coś w buforze */
        if (twait(sem_full, 1) != 0) continue;
        if (!shm->running) { sem_post(sem_full); break; }

        sem_wait(sem_mtx);
        strncpy(task, shm->data[shm->head], TASK_LEN);
        shm->head = (shm->head + 1) % shm->capacity;
        shm->count--;
        shm->consumed++;
        sem_post(sem_mtx);
        sem_post(sem_empty);  /* zwolniliśmy slot */

        /* wypisz znak po znaku, 0.3s przerwy */
        printf("[C%d] czytam: \"", id); fflush(stdout);
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

    printf("[main] N=%d producentów, M=%d konsumentów, K=%d, czas=%ds\n",
           N, M, K, SIM_TIME);

    /* shared memory */
    shm_unlink(SHM_NAME);
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedMem));
    shm = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    memset(shm, 0, sizeof(SharedMem));
    shm->capacity = K;
    shm->running  = 1;

    /* semafory: mutex=1, empty=K (K wolnych slotów), full=0 */
    sem_unlink(SEM_MTX); sem_unlink(SEM_EMPTY); sem_unlink(SEM_FULL);
    sem_mtx   = sem_open(SEM_MTX,   O_CREAT, 0666, 1);
    sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    /* forkuj producentów i konsumentów */
    for (int i = 0; i < N; i++) {
        if (fork() == 0) { open_sems(); producer_loop(i); }
    }
    for (int i = 0; i < M; i++) {
        if (fork() == 0) { open_sems(); consumer_loop(i); }
    }

    /* rodzic czeka, potem zatrzymuje system */
    sleep(SIM_TIME);
    printf("\n[main] stop!\n");
    shm->running = 0;

    /* odblokuj procesy czekające na semaforze */
    for (int i = 0; i < N + M + 2; i++) {
        sem_post(sem_full); sem_post(sem_empty);
    }

    while (wait(NULL) > 0);

    /* zapisz statystyki PRZED munmap – po odmapowaniu shm jest niedostępne! */
    int total_produced = shm->produced;
    int total_consumed = shm->consumed;

    /* sprzątanie */
    munmap(shm, sizeof(SharedMem)); shm_unlink(SHM_NAME);
    sem_close(sem_mtx);   sem_unlink(SEM_MTX);
    sem_close(sem_empty); sem_unlink(SEM_EMPTY);
    sem_close(sem_full);  sem_unlink(SEM_FULL);

    printf("[main] Wyprodukowano: %d | Skonsumowano: %d\n",
           total_produced, total_consumed);
    return 0;
}
