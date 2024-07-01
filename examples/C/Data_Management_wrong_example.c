#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int a[size], b[size], c[size];

// Structured data region
    #pragma acc data create(a, b) copyout(c)
    {
        #pragma acc parallel loop present(a, b)
        for(int i=0; i<size; ++i){
            a[i] = i;
            b[i] = 2*i;
        }
        // We update an element of the array on the CPU 
        a[14] = 78324;

        #pragma acc parallel loop present(b, c) copyin(a)
        for (int i=0; i<size; ++i){
            c[i] = a[i]+b[i];
        }
    }
    printf("value at position 14: %d\n", c[14]); 
}

