#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Вывод PID, PPID и времени в формате ЧЧ:ММ:СС
void print_process_info(const char *role)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%s] PID: %d, PPID: %d, Время: %02d:%02d:%02d\n",
           role, getpid(), getppid(), t->tm_hour, t->tm_min, t->tm_sec);
    fflush(stdout);
}

int main()
{
    pid_t pid1, pid2;

    print_process_info("РОДИТЕЛЬ (до fork)");

    // 1-й дочерний процесс
    pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork 1 failed");
        return 1;
    }

    if (pid1 == 0)
    {
        print_process_info("ДОЧЕРНИЙ-1");
        printf("--- Вариант 18: статистика подсистемы ввода-вывода ---\n");
        system("iostat"); // Если iostat не установлен: sudo apt install sysstat
        _exit(0);
    }

    // 2-й дочерний процесс
    pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork 2 failed");
        return 1;
    }

    if (pid2 == 0)
    {
        print_process_info("ДОЧЕРНИЙ-2");
        _exit(0);
    }

    // Родительский процесс
    print_process_info("РОДИТЕЛЬ (после fork)");
    printf("\n--- Вывод команды ps -x ---\n");
    system("ps -x");

    // Ожидание завершения дочерних процессов
    wait(NULL);
    wait(NULL);
    printf("\n[РОДИТЕЛЬ] Оба дочерних процесса завершены. Ресурсы освобождены.\n");
    return 0;
}