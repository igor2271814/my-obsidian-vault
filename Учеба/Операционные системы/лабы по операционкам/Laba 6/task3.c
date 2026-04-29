#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

// Флаг, изменяемый в обработчике сигнала
volatile sig_atomic_t start_work = 0;

// Обработчик сигнала
void signal_handler(int sig)
{
    if (sig == SIGUSR1)
    {
        start_work = 1;
    }
}

// Функция потока-работника
void *worker(void *arg)
{
    printf("[Работник] Жду сигнал SIGUSR1 для старта...\n");

    // Блокировка потока до прихода сигнала
    while (!start_work)
    {
        pause(); // Приостанавливает поток до выполнения обработчика
    }

    printf("[Работник] Сигнал получен! Начинаю обработку данных...\n");
    sleep(2); // Имитация полезной нагрузки
    printf("[Работник] Обработка завершена.\n");
    return NULL;
}

int main()
{
    // Настройка обработчика сигнала через sigaction (современный и безопасный способ)
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask); // Не блокировать другие сигналы во время обработки
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    pthread_t tid;
    pthread_create(&tid, NULL, worker, NULL);

    // Главный поток "готовит ресурсы"
    sleep(1);
    printf("[Главный] Ресурсы подготовлены. Отправляю сигнал работнику...\n");

    // Отправка сигнала конкретному потоку
    pthread_kill(tid, SIGUSR1);

    pthread_join(tid, NULL);
    printf("[Главный] Все потоки завершены. Программа работает корректно.\n");
    return 0;
}