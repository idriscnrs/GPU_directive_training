#include <stdio.h>
int main(void)
{
    int size = 100000;
    int array[size];
    #pragma acc parallel
#pragma omp target teams
    {
    #pragma acc loop
#pragma omp loop
    for (int i=0; i<size; ++i)
        array[i] = 2 * i;
    }
    printf("%d", array[12]);
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Get_started_init_array_solution_acc_2.c
