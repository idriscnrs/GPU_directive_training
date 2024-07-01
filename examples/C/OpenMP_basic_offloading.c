#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int size = 100000000;
    double* array = (double*) malloc(size*sizeof(double));

// We need to explicitly manage transfers in C with NVIDIA compilers
// otherwise we get a runtime error (as of nvhpc 21.9)
#pragma omp target teams distribute parallel for simd map(from:array[0:size])
    for (int i=0; i<size; ++i)
        array[i] = (double) i;

    printf("array[42] = %f\n", array[42]);
    free(array);
    return 0;
}
