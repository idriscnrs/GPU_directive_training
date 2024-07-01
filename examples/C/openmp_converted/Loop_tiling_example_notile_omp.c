#include <stdio.h>
int main(void)
{ 
    int ny = 10000;
    int nx = 10000;
    int idxA, idxB;
    int a[nx*ny], b[nx*ny];

// Structured data region
    #pragma acc data copyout(a, b)
#pragma omp target data map(from:a,b)
    {
        #pragma acc parallel loop present(a, b)
#pragma omp target teams loop map(present,alloc:a,b)
        for(int i=0; i<nx*ny; ++i)
            a[i] = i;
           
        #pragma acc parallel loop present(a, b)
#pragma omp target teams loop map(present,alloc:a,b)
        for (int i=0; i<nx; ++i){
            for (int j=0; j<ny; ++j){
                idxB = j + i*nx;
                idxA = i + j*ny;
                b[idxB] = a[idxA];
            }
        }
    }
 
    fprintf(stderr,"A[%d]=%d , A[%d]=%d\n",1+2*nx,a[1+2*nx],2+1*ny,a[2+1*ny] );
    fprintf(stderr,"A[%d]=%d , A[%d]=%d\n",1+2*nx,b[1+2*nx],2+1*ny,b[2+1*ny] );
}

// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Loop_tiling_example_notile.c
