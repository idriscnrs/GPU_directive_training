#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    double* s;
    double* c;
} Array;

Array* allocate_array(size_t size);

Array* allocate_array(size_t size)
{
    Array* arr = (Array*) malloc(sizeof(Array));
    arr->s = (double*) malloc(size*sizeof(double));
    arr->c = (double*) malloc(size*sizeof(double));
    return arr;
}

int main(void)
{
    int size=1e5;
    double sum[size];

    Array* vec;
    vec = allocate_array(size);

    #pragma acc data create(vec, vec->s[:size], vec->c[:size]) copyout(sum)
#pragma omp target data map(from:sum) map(alloc:vec,vec->s[:size],vec->c[:size])
    {
    #pragma acc parallel
#pragma omp target teams
    {
        #pragma acc loop
#pragma omp loop
        for (int i=0; i<size;++i)
        {
            vec->s[i] = sin(i*M_PI/size)*sin(i*M_PI/size);
            vec->c[i] = cos(i*M_PI/size)*cos(i*M_PI/size);
        }    
    }
    #pragma acc parallel
#pragma omp target teams
    {
        #pragma acc loop
#pragma omp loop
        for (int i=1; i<size ; ++i)
            sum[i] = vec->s[i] + vec->c[size - i]; 
    }
    }// end of structured data region
    printf("sum[42] = %f\n", sum[42]);
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Deep_copy_example.c
