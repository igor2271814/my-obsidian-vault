#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

// Функция вывода информации
void print_info(const char *role)
{
    time_t now = time(nullptr);
    tm *t = localtime(&now);

    printf("[%s] ThreadID: %lu, PID: %d, PPID: %d, Время: %02d:%02d:%02d\n",
           role,
           (unsigned long)pthread_self(),
           getpid(),
           getppid(),
           t->tm_hour, t->tm_min, t->tm_sec);
    fflush(stdout);
}

// Функция, выполняемая потоком
void *thread_func(void *arg)
{
    // Безопасное приведение обратно к строке
    const char *name = static_cast<const char *>(arg);
    print_info(name);
    pthread_exit(nullptr);
}

int main()
{
    pthread_t th1, th2;
    print_info("ГЛАВНЫЙ ПОТОК");

    char name1[] = "ПОТОК 1";
    char name2[] = "ПОТОК 2";

    if (pthread_create(&th1, nullptr, thread_func, name1) != 0)
    {
        perror("Ошибка создания потока 1");
        return EXIT_FAILURE;
    }

    if (pthread_create(&th2, nullptr, thread_func, name2) != 0)
    {
        perror("Ошибка создания потока 2");
        return EXIT_FAILURE;
    }

    // Ожидание завершения дочерних потоков
    pthread_join(th1, nullptr);
    pthread_join(th2, nullptr);

    printf("[ГЛАВНЫЙ] Оба дочерних потока завершены.\n");
    return EXIT_SUCCESS;
}