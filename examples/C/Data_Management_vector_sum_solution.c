#include <stdio.h>
#include <stdlib.h>
int main(void){
    int size = 10000;
    int a[size], b[size], c[size];

    #pragma acc parallel loop copyout(a, b)
    for(int i=0; i<size; ++i){
        a[i] = i;
        b[i] = 2*i;
    }

    #pragma acc parallel loop copyin(a, b) copyout(c)
    for (int i=0; i<size; ++i){
        c[i] = a[i]+b[i];
    }

    printf("value at position 14: %d\n", c[14]); 
}

