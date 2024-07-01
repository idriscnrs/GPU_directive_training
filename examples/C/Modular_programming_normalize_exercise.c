#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void rand_init(double* array, size_t array_size)
{
     srand((unsigned) 12345900);
     for (size_t i=0; i<array_size; ++i)
         array[i] = 2.*((double)rand()/RAND_MAX -0.5);
}

double dot_prod(double* restrict u, double* restrict v, size_t size)
{
    double N = 0.0;
    for (size_t i=0; i<size; ++i)
        N += u[i]*v[i];
    return N;
}

double check(double* restrict A, size_t num_rows, size_t num_cols)
{
    /* Sum the dot products of the rows of A */
    double s = 0.0;
    for (size_t i=0; i<num_rows; ++i)
    {   
        double d = dot_prod(&A[i*num_cols], &A[i*num_cols], num_cols);
        s += d;
    }   
    return s;
}

void normalize(double* restrict v, size_t size)
{
    double N_inv = 1.0/sqrt(dot_prod(v, v, size));
    for (size_t i=0;i<size;++i)
        v[i] *= N_inv;
}

int main(void)
{
    size_t size = 3000;
    double* restrict A = (double*) malloc(size*size*sizeof(double));
    
    rand_init(A, size*size);
    
    /* virtual loop not to be parallelized (just here to increase compute time)*/
    for (int n=0;n<1000;n++)
    {
        for (size_t i=0; i<size; ++i)
            normalize(&A[i*size], size);
    }
    double c = check(A, size, size);
    
    printf("A(%d x %d), check=%10.5e\n", size, size, c);
    free(A);
}
