#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void inplace_sum(double* A, double* B, size_t size)
{
    #pragma acc parallel loop present(A[0:size], B[0:size])
    for (size_t i=0; i<size; ++i)
        A[i] += B[i]; 
}

int main(void)
{
    size_t size = (size_t) 1e9;
    double* A = (double*) malloc(size*sizeof(double));
    double* B = (double*) malloc(size*sizeof(double));
    double sum = 0.0;

    #pragma acc data create(A[0:size], B[0:size])
    {   
        #pragma acc parallel loop present(A[0:size], B[0:size])
        for (size_t i=0; i<size; ++i)
        {   
            A[i] = sin(M_PI*(double)i/(double)size)*sin(M_PI*(double)i/(double)size);
            B[i] = cos(M_PI*(double)i/(double)size)*cos(M_PI*(double)i/(double)size);
        }   

        inplace_sum(A, B, size);

        #pragma acc parallel loop present(A[0:size], B[0:size]) reduction(+:sum)
        for (size_t i=0; i<size; ++i)
            sum += A[i];
    }   
    printf("This should be close to 1.0: %f\n", sum/(double) size);
    return 0;
}
