#include <stdio.h>
#include <stdlib.h>
double* init(size_t size){
    double* array = (double*) malloc(size*sizeof(double));
    #pragma acc enter data create(array[0:size])
#pragma omp target enter data map(alloc:array[0:size])
    return array;
}

int main(void){
    size_t size = 100000;
    double* array = init(size);
    #pragma acc parallel loop present(array[0:size])
#pragma omp target teams loop map(present,alloc:array[0:size])
    for (size_t i=0; i<size; ++i)
        array[i] = i;
    printf("%f\n", array[42]);
    #pragma acc exit data delete(array[0:size])
#pragma omp target exit data map(delete:array[0:size])
    free(array);
}


// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_unstructured_solution.c
