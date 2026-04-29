#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4
#define ITERATIONS 10000

// Глобальный мьютекс и защищаемая переменная
pthread_mutex_t cash_mutex;
int total_revenue = 0;

void *sell_tickets(void *arg)
{
    int id = *(int *)arg;
    free(arg);

    for (int i = 0; i < ITERATIONS; i++)
    {
        // Вход в критическую секцию
        pthread_mutex_lock(&cash_mutex);

        // Только один поток может выполнять эту строку одновременно
        total_revenue += 1;

        // Выход из критической секции
        pthread_mutex_unlock(&cash_mutex);
    }

    printf("[Кассир %d] Завершил смену.\n", id);
    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];

    // Инициализация мьютекса с атрибутами по умолчанию
    pthread_mutex_init(&cash_mutex, NULL);

    printf("Начало работы. Ожидаемая выручка: %d\n", NUM_THREADS * ITERATIONS);
    printf("-----------------------------------------------\n");

    // Запуск потоков
    for (int i = 0; i < NUM_THREADS; i++)
    {
        int *tid = malloc(sizeof(int));
        *tid = i + 1;
        pthread_create(&threads[i], NULL, sell_tickets, tid);
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("-----------------------------------------------\n");
    printf("Итоговая выручка: %d\n", total_revenue);

    // Очистка ресурсов мьютекса
    pthread_mutex_destroy(&cash_mutex);
    return 0;
}