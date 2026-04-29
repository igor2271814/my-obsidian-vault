#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Общий семафор для потоков внутри одного процесса
sem_t coffee_machine;
#define NUM_THREADS 5
    
void *use_coffee_machine(void *arg)
{
    int id = *(int *)arg;
    free(arg); // Освобождаем память, выделенную в main

    printf("[Поток %d] Подхожу к кофемашине и жду...\n", id);
    sem_wait(&coffee_machine); // Захват ресурса (уменьшает счётчик на 1)

    printf("[Поток %d] Готовлю кофе...\n", id);
    sleep(2); // Имитация времени приготовления

    printf("[Поток %d] Кофе готов. Освобождаю кофемашину.\n", id);
    sem_post(&coffee_machine); // Освобождение ресурса (увеличивает счётчик на 1)

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];

    // Инициализируем семафор: 0 = для потоков внутри процесса, 2 = максимум 2 потока одновременно
    if (sem_init(&coffee_machine, 0, 2) != 0)
    {
        perror("Ошибка инициализации семафора");
        return 1;
    }

    printf("Кофемашина запущена. Максимум 2 потока одновременно.\n");
    printf("-----------------------------------------------\n");

    for (int i = 0; i < NUM_THREADS; i++)
    {
        int *tid = malloc(sizeof(int));
        *tid = i + 1;
        pthread_create(&threads[i], NULL, use_coffee_machine, tid);
        usleep(300000); // Небольшая задержка для наглядности порядка запуска
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&coffee_machine);
    printf("-----------------------------------------------\n");
    printf("Все сотрудники выпили кофе. Работа завершена.\n");
    return 0;
}