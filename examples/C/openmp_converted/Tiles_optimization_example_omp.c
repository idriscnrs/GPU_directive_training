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
           
        #pragma acc parallel loop present(a, b) tile(32,32)
#pragma omp target teams loop map(present,alloc:a,b)
        for (int i=0; i<nx; ++i){
            for (int j=0; j<ny; ++j){
                idxB = j + (i)*nx;
                idxA = i + (j)*ny;
                b[idxB] = a[idxA];
            }
        }
    }
// End of structured data region
/*
    fprintf(stderr,"A: \n");
    for(int i=0; i<nx*ny; ++i) fprintf(stderr,"%d\n",a[i]);
    fprintf(stderr,"\n");
    fprintf(stderr,"B: \n");
    for(int i=0; i<nx*ny; ++i) fprintf(stderr,"%d\n",b[i]);
*/    
}

// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Tiles_optimization_example.c
