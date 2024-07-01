#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int size = 10;
    double* array = (double*) malloc(size*sizeof(double));
    double  scalar;

    scalar = 1000.0;
#pragma omp target teams distribute parallel for
    for (int i=0; i<size; ++i)
        array[i] = (double)i + scalar;

    for (int i=0; i<size; ++i)
        printf("%lf\n",array[i]);

    scalar = -1000.1;

#pragma omp target teams distribute parallel for
    for (int i=0; i<size; ++i)
        array[i] = (double)i + scalar;

    for (int i=0; i<size; ++i)
        printf("%lf\n",array[i]);	    

    free(array);
    return 0;
}
