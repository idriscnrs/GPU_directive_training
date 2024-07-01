#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int a[size], b[size], c[size];

// Structured data region
    #pragma acc data create(a, b) copyout(c)
#pragma omp target data map(from:c) map(alloc:a,b)
    {
        #pragma acc parallel loop present(a, b)
#pragma omp target teams loop map(present,alloc:a,b)
        for(int i=0; i<size; ++i){
            a[i] = i;
            b[i] = 2*i;
        }
        // We update an element of the array on the CPU 
        a[14] = 78324;

        #pragma acc parallel loop present(b, c) copyin(a)
#pragma omp target teams loop map(to:a) map(present,alloc:b,c)
        for (int i=0; i<size; ++i){
            c[i] = a[i]+b[i];
        }
    }
    printf("value at position 14: %d\n", c[14]); 
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_wrong_example.c
