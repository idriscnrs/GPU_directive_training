#include <stdio.h>
#include <stdlib.h>
double* init(size_t size){
    double* array = (double*) malloc(size*sizeof(double));
    return array;
}

int main(void){
    size_t size = 100000;
    double* array = init(size);
    for (size_t i=0; i<size; ++i)
        array[i] = (double)i;
    printf("This should be 42: %f\n", array[42]);
    free(array);
}

