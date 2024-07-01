#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int a[size], b[size], c[size];

    #pragma acc parallel loop copyout(a, b)
#pragma omp target teams loop map(from:a,b)
    for(int i=0; i<size; ++i){
        a[i] = i;
        b[i] = 2*i;
    }

    #pragma acc parallel loop copyin(a, b) copyout(c)
#pragma omp target teams loop map(to:a,b) map(from:c)
    for (int i=0; i<size; ++i){
        c[i] = a[i]+b[i];
    }

    printf("value at position 14: %d\n", c[14]); 
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_vector_sum_solution.c
