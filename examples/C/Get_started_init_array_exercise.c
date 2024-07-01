#include <stdio.h>
int main(void)
{
    int size = 100000;
    int array[size];
    for (int i=0; i<size; ++i)
        array[i] = 2 * i;
    printf("%d", array[21]);
}

