#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

int summ(int a, int b)
{
    int c = a + b;
    return c;
}
int main()
{
    int a = 5, b = 6, c = 0;
    c = summ(a, b);
    printf("Сумма %d и %d равна %d\n", a, b, c);
    return 0;
}