#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define SEM_NAME "/lab8_sync"
#define BUFFER_FILE "buffer.txt"

int main()
{
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0644, 0);
    if (sem == SEM_FAILED)
    {
        perror("sem_open consumer");
        return 1;
    }

    printf("[Consumer] Запуск. Ожидаю данные через семафор %s...\n", SEM_NAME);

    for (int i = 0; i < 5; i++)
    {
        // Блокировка до сигнала от producer
        sem_wait(sem);
        printf("[Consumer] Получил сигнал (sem_wait). Читаю файл...\n");

        // Чтение данных из общего файла
        FILE *f = fopen(BUFFER_FILE, "r");
        if (!f)
        {
            perror("fopen");
            continue;
        }
        int value;
        fscanf(f, "%d", &value);
        fclose(f);
        printf("[Consumer] Прочитал данные: %d\n", value);
    }

    // Очистка: удаляем семафор из системы (последний пользователь)
    sem_close(sem);
    sem_unlink(SEM_NAME);
    printf("[Consumer] Завершён. Семафор %s удалён из системы.\n", SEM_NAME);
    return 0;
}