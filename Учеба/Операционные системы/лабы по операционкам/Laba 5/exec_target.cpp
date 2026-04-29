#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

void print_info(const char *role)
{
    time_t now = time(nullptr);
    tm *t = localtime(&now);

    printf("[%s] PID: %d, ThreadID: %lu, Время: %02d:%02d:%02d\n",
           role, getpid(), (unsigned long)pthread_self(),
           t->tm_hour, t->tm_min, t->tm_sec);
    fflush(stdout);
}

void *thread_func(void *arg)
{
    const char *name = static_cast<const char *>(arg);
    print_info(name);
    pthread_exit(nullptr);
}

int main()
{
    pthread_t th1, th2;
    print_info("EXEC-MAIN");

    char name1[] = "EXEC-THREAD-1";
    char name2[] = "EXEC-THREAD-2";

    if (pthread_create(&th1, nullptr, thread_func, name1) != 0)
    {
        perror("pthread_create 1");
        return EXIT_FAILURE;
    }
    if (pthread_create(&th2, nullptr, thread_func, name2) != 0)
    {
        perror("pthread_create 2");
        return EXIT_FAILURE;
    }

    pthread_join(th1, nullptr);
    pthread_join(th2, nullptr);

    printf("[EXEC-MAIN] Потоки завершены.\n");
    return EXIT_SUCCESS;
}