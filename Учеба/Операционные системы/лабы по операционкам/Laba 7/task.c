#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#define SIZE 4
#define WORKERS 2
#define SHM_PREFIX "/lr9_shm_"
#define SOCK_PREFIX "/tmp/lr9_sock_"

double A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];

typedef struct
{
    int r, c;
} Task;
typedef struct
{
    int r, c;
    double val;
} Result;

// --- Вспомогательные функции ---
double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void init_matrices()
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
        {
            A[i][j] = (i + 1) * (j + 1);
            B[i][j] = (i * j) + 1.0;
            C[i][j] = 0.0;
        }
}

double calc_elem(int r, int c)
{
    double sum = 0.0;
    for (int k = 0; k < SIZE; k++)
        sum += A[r][k] * B[k][c];
    return sum;
}

void print_result()
{
    printf("\nРезультирующая матрица C:\n");
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            printf("%8.2f ", C[i][j]);
        printf("\n");
    }
}

// 1. РЕАЛИЗАЦИЯ ЧЕРЕЗ НЕИМЕНОВАННЫЕ КАНАЛЫ
void worker_pipe(int rd_fd, int wr_fd)
{
    Task t;
    while (read(rd_fd, &t, sizeof(t)) == sizeof(t))
    {
        if (t.r == -1 && t.c == -1)
            break; // Сигнал завершения
        Result res = {t.r, t.c, calc_elem(t.r, t.c)};
        write(wr_fd, &res, sizeof(res));
    }
    close(rd_fd);
    close(wr_fd);
    exit(0);
}

void run_pipe()
{
    int to_child[WORKERS][2], from_child[WORKERS][2];
    pid_t pids[WORKERS];

    for (int i = 0; i < WORKERS; i++)
    {
        if (pipe(to_child[i]) == -1 || pipe(from_child[i]) == -1)
        {
            perror("pipe");
            exit(1);
        }
        pids[i] = fork();
        if (pids[i] == 0)
        { // Ведомый процесс
            close(to_child[i][1]);
            close(from_child[i][0]);
            worker_pipe(to_child[i][0], from_child[i][1]);
        }
    }

    // Ведущий процесс
    for (int i = 0; i < WORKERS; i++)
    {
        close(to_child[i][0]);
        close(from_child[i][1]);
    }

    // Рассылка заданий (циклически)
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
        {
            int wid = (r * SIZE + c) % WORKERS;
            Task t = {r, c};
            write(to_child[wid][1], &t, sizeof(t));
        }

    // Сигнал завершения
    Task end_task = {-1, -1};
    for (int i = 0; i < WORKERS; i++)
        write(to_child[i][1], &end_task, sizeof(end_task));

    // Чтение результатов через select
    int active_fds = WORKERS;
    fd_set readfds;
    while (active_fds > 0)
    {
        FD_ZERO(&readfds);
        int max_fd = 0;
        for (int i = 0; i < WORKERS; i++)
        {
            FD_SET(from_child[i][0], &readfds);
            if (from_child[i][0] > max_fd)
                max_fd = from_child[i][0];
        }
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        for (int i = 0; i < WORKERS; i++)
        {
            if (FD_ISSET(from_child[i][0], &readfds))
            {
                Result res;
                if (read(from_child[i][0], &res, sizeof(res)) == sizeof(res))
                {
                    C[res.r][res.c] = res.val;
                }
                else
                {
                    FD_CLR(from_child[i][0], &readfds);
                    active_fds--;
                }
            }
        }
    }

    for (int i = 0; i < WORKERS; i++)
        close(to_child[i][1]), close(from_child[i][0]);
    for (int i = 0; i < WORKERS; i++)
        wait(NULL);
}

// 2. РЕАЛИЗАЦИЯ ЧЕРЕЗ РАЗДЕЛЯЕМУЮ ПАМЯТЬ
typedef struct
{
    Task t;
    Result r;
    volatile int task_ready;
    volatile int result_ready;
} ShmBlock;

void worker_shm(int id, char *name)
{
    int fd = shm_open(name, O_RDWR, 0666);
    ShmBlock *shm = mmap(NULL, sizeof(ShmBlock), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    while (1)
    {
        while (!shm->task_ready)
            usleep(500); // Ожидание задания
        if (shm->t.r == -1 && shm->t.c == -1)
            break;
        shm->r.val = calc_elem(shm->t.r, shm->t.c);
        shm->r.r = shm->t.r;
        shm->r.c = shm->t.c;
        shm->result_ready = 1;
    }
    munmap(shm, sizeof(ShmBlock));
    exit(0);
}

void run_shm()
{
    pid_t pids[WORKERS];
    ShmBlock *shms[WORKERS];
    int fds[WORKERS];
    char name[32];

    for (int i = 0; i < WORKERS; i++)
    {
        snprintf(name, sizeof(name), "%s%d", SHM_PREFIX, i);
        fds[i] = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(fds[i], sizeof(ShmBlock));
        shms[i] = mmap(NULL, sizeof(ShmBlock), PROT_READ | PROT_WRITE, MAP_SHARED, fds[i], 0);
        shms[i]->task_ready = 0;
        shms[i]->result_ready = 0;

        pids[i] = fork();
        if (pids[i] == 0)
        {
            worker_shm(i, name);
        }
    }

    // Ведущий: распределение и сбор
    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            int wid = (r * SIZE + c) % WORKERS;
            shms[wid]->t = (Task){r, c};
            shms[wid]->task_ready = 1;

            while (!shms[wid]->result_ready)
                usleep(500);
            C[r][c] = shms[wid]->r.val;
            shms[wid]->task_ready = 0;
            shms[wid]->result_ready = 0;
        }
    }

    // Завершение
    Task end = {-1, -1};
    for (int i = 0; i < WORKERS; i++)
    {
        shms[i]->t = end;
        shms[i]->task_ready = 1;
    }
    for (int i = 0; i < WORKERS; i++)
        wait(NULL), munmap(shms[i], sizeof(ShmBlock)), close(fds[i]);
    for (int i = 0; i < WORKERS; i++)
    {
        snprintf(name, sizeof(name), "%s%d", SHM_PREFIX, i);
        shm_unlink(name);
    }
}

// 3. РЕАЛИЗАЦИЯ ЧЕРЕЗ СОКЕТЫ (AF_UNIX, DGRAM)
void worker_socket(int id, char *sock_path)
{
    struct sockaddr_un addr;
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);
    unlink(sock_path);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    char buf[1024];
    struct sockaddr_un client_addr;
    socklen_t len = sizeof(client_addr);

    while (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &len) > 0)
    {
        Task *t = (Task *)buf;
        if (t->r == -1 && t->c == -1)
            break;
        Result res = {t->r, t->c, calc_elem(t->r, t->c)};
        sendto(sock, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, len);
    }
    close(sock);
    unlink(sock_path);
    exit(0);
}

void run_socket()
{
    pid_t pids[WORKERS];
    char worker_path[64];
    char master_path[] = "/tmp/lr9_master.sock";
    struct sockaddr_un master_addr, worker_addr;

    // Создаем сокет ведущего
    int master_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    memset(&master_addr, 0, sizeof(master_addr));
    master_addr.sun_family = AF_UNIX;
    strcpy(master_addr.sun_path, master_path);
    unlink(master_path);
    bind(master_sock, (struct sockaddr *)&master_addr, sizeof(master_addr));

    for (int i = 0; i < WORKERS; i++)
    {
        snprintf(worker_path, sizeof(worker_path), "%s%d.sock", SOCK_PREFIX, i);
        pids[i] = fork();
        if (pids[i] == 0)
            worker_socket(i, worker_path);
    }

    // Рассылка и чтение
    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            int wid = (r * SIZE + c) % WORKERS;
            snprintf(worker_path, sizeof(worker_path), "%s%d.sock", SOCK_PREFIX, wid);

            Task t = {r, c};
            memset(&worker_addr, 0, sizeof(worker_addr));
            worker_addr.sun_family = AF_UNIX;
            strcpy(worker_addr.sun_path, worker_path);

            sendto(master_sock, &t, sizeof(t), 0, (struct sockaddr *)&worker_addr, sizeof(worker_addr));

            Result res;
            recvfrom(master_sock, &res, sizeof(res), 0, NULL, NULL);
            C[res.r][res.c] = res.val;
        }
    }

    // Завершение
    Task end = {-1, -1};
    for (int i = 0; i < WORKERS; i++)
    {
        snprintf(worker_path, sizeof(worker_path), "%s%d.sock", SOCK_PREFIX, i);
        memset(&worker_addr, 0, sizeof(worker_addr));
        worker_addr.sun_family = AF_UNIX;
        strcpy(worker_addr.sun_path, worker_path);
        sendto(master_sock, &end, sizeof(end), 0, (struct sockaddr *)&worker_addr, sizeof(worker_addr));
    }
    for (int i = 0; i < WORKERS; i++)
        wait(NULL);
    close(master_sock);
    unlink(master_path);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Использование: %s <pipe|shm|socket>\n", argv[0]);
        return 1;
    }

    init_matrices();
    printf("Матрица A:\n");
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            printf("%4.0f ", A[i][j]);
        printf("\n");
    }
    printf("Матрица B:\n");
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            printf("%4.0f ", B[i][j]);
        printf("\n");
    }

    double t_start = get_time();
    if (strcmp(argv[1], "pipe") == 0)
        run_pipe();
    else if (strcmp(argv[1], "shm") == 0)
        run_shm();
    else if (strcmp(argv[1], "socket") == 0)
        run_socket();
    else
    {
        fprintf(stderr, "Неизвестный режим\n");
        return 1;
    }
    double t_end = get_time();

    print_result();
    printf("Время выполнения (%s): %.6f сек\n", argv[1], t_end - t_start);
    return 0;
}