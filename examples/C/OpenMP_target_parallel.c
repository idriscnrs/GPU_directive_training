#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t size = 10000000;
    double* array = (double*) malloc(size*sizeof(double));

#pragma omp target enter data map(alloc:array[0:size])
#pragma omp target parallel
    {
    for (int i=0; i<size; ++i)
        array[i] = (double)i;
    }
#pragma omp target update from(array[0:size])
}
