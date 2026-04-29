#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <ctime>

void print_info(const char *role)
{
    time_t now = time(nullptr);
    tm *t = localtime(&now);

    printf("[%s] PID: %d, PPID: %d, Время: %02d:%02d:%02d\n",
           role, getpid(), getppid(),
           t->tm_hour, t->tm_min, t->tm_sec);
    fflush(stdout);
}

int main()
{
    pid_t pid1, pid2;
    print_info("РОДИТЕЛЬ");

    // 1-й дочерний процесс (как в задании 2)
    pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork 1");
        return EXIT_FAILURE;
    }
    if (pid1 == 0)
    {
        print_info("ДОЧЕРНИЙ-1");
        printf("--- Статистика подсистемы ввода-вывода ---\n");
        system("iostat");
        _exit(EXIT_SUCCESS);
    }

    // 2-й дочерний процесс -> будет заменён через exec
    pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork 2");
        return EXIT_FAILURE;
    }
    if (pid2 == 0)
    {
        print_info("ДОЧЕРНИЙ-2 (до exec)");

        // Заменяем адресное пространство текущего процесса новым исполняемым файлом
        execl("./exec_target", "exec_target", (char *)NULL);

        // Если execl вернулась — произошла ошибка (например, файл не найден)
        perror("exec failed");
        _exit(EXIT_FAILURE);
    }

    // Родительский процесс
    print_info("РОДИТЕЛЬ (ожидание)");
    printf("\n--- Список процессов (ps -x) ---\n");
    system("ps -x");

    wait(nullptr);
    wait(nullptr);
    printf("\n[РОДИТЕЛЬ] Все дочерние процессы завершены.\n");
    return EXIT_SUCCESS;
}