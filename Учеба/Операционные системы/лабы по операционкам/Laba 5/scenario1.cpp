#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        // Дочерний процесс
        printf("Child: Я дочерний процесс\n");
        _exit(0);
    }
    else
    {
        // Родительский процесс
        printf("Parent: Я родительский процесс\n");
        wait(NULL);
    }
    return 0;
}