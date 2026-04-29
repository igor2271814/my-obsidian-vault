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
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0644, 0); // Начальное значение 0 = буфер пуст
    if (sem == SEM_FAILED)
    {
        perror("sem_open producer");
        return 1;
    }

    printf("[Producer] Запуск. Именованный семафор: %s\n", SEM_NAME);

    for (int i = 1; i <= 5; i++)
    {
        // Запись данных в общий файл
        FILE *f = fopen(BUFFER_FILE, "w");
        if (!f)
        {
            perror("fopen");
            break;
        }
        fprintf(f, "%d\n", i);
        fclose(f);
        printf("[Producer] Записал данные: %d\n", i);

        // Сигнал потребителю: данные готовы
        sem_post(sem);
        printf("[Producer] Отправил сигнал (sem_post)\n");

        sleep(1); // Имитация интервала между записями
    }

    sem_close(sem);
    printf("[Producer] Завершён. Не удаляйте семафор вручную — это сделает consumer.\n");
    return 0;
}