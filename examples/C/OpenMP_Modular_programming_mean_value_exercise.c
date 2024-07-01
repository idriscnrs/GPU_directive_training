#include <stdio.h>
#include <stdlib.h>
double mean_value(double* array, size_t array_size){
    double sum = 0.0;
    for(size_t i=0; i<array_size; ++i)
        sum += array[i];
    return sum/array_size;
}

void rand_init(double* array, size_t array_size)
{
     srand((unsigned) 12345900);
     for (size_t i=0; i<array_size; ++i)
         array[i] = 2.*((double)rand()/RAND_MAX -0.5);
}

void iterate(double* array, size_t array_size, size_t cell_size)
{
    double local_mean;
    for (size_t i = cell_size/2; i< array_size-cell_size/2; ++i)
    {
        local_mean = mean_value(&array[i-cell_size/2], cell_size);
        if (local_mean < 0.)
            array[i] += 0.1;
        else if (local_mean > 0.)
            array[i] -= 0.1;
    }
}

int main(void){
    size_t num_cols = 10000;
    size_t num_rows = 3000;

    double* table = (double*) malloc(num_rows*num_cols*sizeof(double)); 
    double* mean_values = (double*) malloc(num_rows*sizeof(double));
    // We initialize the first row with random values between -1 and 1
    rand_init(table, num_cols);

    for (size_t i=1; i<num_rows; ++i)
       iterate(&table[i*num_cols], num_cols, 32); 
    
    for (size_t i=0; i<num_rows; ++i) 
    {
        mean_values[i] = mean_value(&(table[i*num_cols]), num_cols);
    }

    for (size_t i=0; i<10; ++i)
        printf("Mean value of row %6d=%10.5f\n", i, table[i]);
    printf("...\n");
    for (size_t i=num_rows-10; i<num_rows; ++i)
        printf("Mean value of row %6d=%10.5f\n", i, table[i]);
    return 0;
}
