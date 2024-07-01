#include <stdio.h>
#include <stdlib.h>
double* create_mat(int dim, int stream)
{
    double* mat = (double*) malloc(dim*dim*sizeof(double));
    #pragma omp enter data map(alloc:mat[0:dim*dim]) nowait depend(out:mat)
    return mat;
}

void init_mat(double* mat, int dim, double diag, int stream)
{
    #pragma acc parallel loop present(mat[0:dim*dim]) async(stream)
    #pragma omp teams distribute parallel for simd collapse(2) nowait depend(inout:mat)
    for (int i=0; i<dim; ++i)
        for (int j=0; j<dim; ++j)
        {
            mat[i*dim+j] = 0.;
        }
    #pragma omp teams distribute parallel for simd nowait depend(in:mat)
    for (int i=0; i<dim; ++i)
        mat[i*dim+i] = diag;
}

int main(void)
{
    int dim = 5000;
    
    double* restrict A = create_mat(dim, 1);
    double* restrict B = create_mat(dim, 2);
    double* restrict C = create_mat(dim, 3);
    
    init_mat(A, dim, 6.0, 1);
    init_mat(B, dim, 7.0, 2);
    init_mat(C, dim, 0.0, 3);

    #pragma omp target teams distribut parallel for simd collaspe(3)	    
    for (int i=0; i<dim; ++i)
        for (int k=0; k<dim; ++k)
            for (int j=0; j<dim; ++j)
            {
                C[i*dim+j] += A[i*dim+k] * B[k*dim+j];
            }
    }
    #pragma omp target exit data map(delete:A,B)
    #pragma omp target exit data map(from:C[:dim*dim])
    printf("Check that value is equal to 42.: %f\n", C[0]);
    return 0;
}

