#include <stdio.h>
#include <stdlib.h>
double* init(size_t size){
    double* array = (double*) malloc(size*sizeof(double));
    #pragma acc enter data create(array[0:size])
    return array;
}

int main(void){
    size_t size = 100000;
    double* array = init(size);
    #pragma acc parallel loop present(array[0:size])
    for (size_t i=0; i<size; ++i)
        array[i] = i;
    printf("%f\n", array[42]);
    #pragma acc exit data delete(array[0:size])
    free(array);
}

