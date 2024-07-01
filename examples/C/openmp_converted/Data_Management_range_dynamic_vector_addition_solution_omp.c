#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int* a = (int*) malloc(size*sizeof(int));
    int* b = (int*) malloc(size*sizeof(int));
    int* c = (int*) malloc(size*sizeof(int));

// Structured data region
    #pragma acc data create(a[:size], b[:size]) copyout(c[:size])
#pragma omp target data map(from:c[:size]) map(alloc:a[:size],b[:size])
    {         
        #pragma acc parallel loop present(a[:size], b[:size])
#pragma omp target teams loop map(present,alloc:a[:size],b[:size])
        for(int i=0; i<size; ++i){
            a[i] = i;
            b[i] = 2*i;
        }

        #pragma acc parallel loop present(a[:size], b[:size], c[:size])
#pragma omp target teams loop map(present,alloc:a[:size],b[:size],c[:size])
        for (int i=0; i<size; ++i){
            c[i] = a[i]+b[i];
        }
    }
    printf("value at position 12: %d\n", c[12]);
}       


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_range_dynamic_vector_addition_solution.c
