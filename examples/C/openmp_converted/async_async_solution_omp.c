#include <stdio.h>
#include <stdlib.h>
double* create_mat(int dim, int stream)
{
    double* mat = (double*) malloc(dim*dim*sizeof(double));
    #pragma acc enter data create(mat[0:dim*dim]) async(stream)
#pragma omp target enter data map(alloc:mat[0:dim*dim]) nowait
    return mat;
}

void init_mat(double* mat, int dim, double diag, int stream)
{
    #pragma acc parallel loop present(mat[0:dim*dim]) async(stream)
#pragma omp target teams loop map(present,alloc:mat[0:dim*dim]) nowait
    for (int i=0; i<dim; ++i)
        #pragma acc loop
#pragma omp loop
        for (int j=0; j<dim; ++j)
        {
            mat[i*dim+j] = 0.;
        }
    #pragma acc parallel loop present(mat[0:dim*dim]) async(stream)
#pragma omp target teams loop map(present,alloc:mat[0:dim*dim]) nowait
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

    #pragma acc parallel present(A[:dim*dim], B[:dim*dim], C[:dim*dim]) wait(1,2,3)
#pragma omp taskwait
#pragma omp target teams map(present,alloc:A[:dim*dim],B[:dim*dim],C[:dim*dim])
    {
    #pragma acc loop gang vector collapse(3)
#pragma omp loop collapse(3)
    for (int i=0; i<dim; ++i)
        for (int k=0; k<dim; ++k)
            for (int j=0; j<dim; ++j)
            {
                #pragma acc atomic update
#pragma omp atomic update
                C[i*dim+j] += A[i*dim+k] * B[k*dim+j];
            }
    }
    #pragma acc exit data delete(A[:dim*dim], B[:dim*dim]) copyout(C[:dim*dim])
#pragma omp target exit data map(from:C[:dim*dim]) map(delete:A[:dim*dim],\
            B[:dim*dim])
    printf("Check that value is equal to 42.: %f\n", C[0]);
    return 0;
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report async_async_solution.c
