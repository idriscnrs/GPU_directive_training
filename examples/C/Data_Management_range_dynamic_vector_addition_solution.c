#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int* a = (int*) malloc(size*sizeof(int));
    int* b = (int*) malloc(size*sizeof(int));
    int* c = (int*) malloc(size*sizeof(int));

// Structured data region
    #pragma acc data create(a[:size], b[:size]) copyout(c[:size])
    {         
        #pragma acc parallel loop present(a[:size], b[:size])
        for(int i=0; i<size; ++i){
            a[i] = i;
            b[i] = 2*i;
        }

        #pragma acc parallel loop present(a[:size], b[:size], c[:size])
        for (int i=0; i<size; ++i){
            c[i] = a[i]+b[i];
        }
    }
    printf("value at position 12: %d\n", c[12]);
}       

