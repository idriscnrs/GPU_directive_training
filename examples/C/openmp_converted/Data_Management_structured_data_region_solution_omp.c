#include <stdio.h>
#include <stdlib.h>
int main(void)
{
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

        #pragma acc parallel loop present(a, b, c)
#pragma omp target teams loop map(present,alloc:a,b,c)
        for (int i=0; i<size; ++i){
            c[i] = a[i]+b[i];
        }
    } // End of structured data region
    printf("value at position 14: %d\n", c[14]); 
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_structured_data_region_solution.c
