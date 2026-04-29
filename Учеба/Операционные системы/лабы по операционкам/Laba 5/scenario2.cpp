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
        // Замещаем себя программой hello_world
        execl("./hello_world", "hello_world", (char *)NULL);
        perror("exec failed");
        _exit(1);
    }
    else
    {
        printf("Parent: Это строка родителя\n");
        wait(NULL);
    }
    return 0;
}