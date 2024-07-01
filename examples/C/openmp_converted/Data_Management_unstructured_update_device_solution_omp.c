#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

double* init(size_t size)
{
    /**
     *  Initialize the array to 0.0
     */
    double* array = (double*) malloc(size*sizeof(double));
    #pragma acc parallel loop
#pragma omp target teams loop
    for (size_t i=0; i<size; ++i)
        array[i] = 0.;
    #pragma acc enter data copyin(array[0:size])
#pragma omp target enter data map(to:array[0:size])
    return array;
}

void gaussian(double* array, size_t size, int start, int end, double variance)
{
    /**
     * Fill an array with the values of gaussian functions
     */
    double range = end - start;
    double step = range/size;
    double position;
    int num_gauss = 300;
    double norm = 1./(variance*sqrt(2*M_PI));
    #pragma acc parallel present(array[0:size])
#pragma omp target teams map(present,alloc:array[0:size])
    {
        #pragma acc loop seq
#pragma omp loop
        for (double g=start; g<=end; g += range/num_gauss)
            #pragma acc loop private(position)
#pragma omp loop private(position)
            for (size_t i=0; i<size; ++i)
            {
                position = start + i*step;
                array[i] += norm * exp(-0.5*(position-g)*(position-g)/(variance*variance));
            }
    }
}

void add_noise(double* array, size_t size, double max_noise)
{
    /**
     *  Take an array and add some noise to the values
     *  max_noise is a fraction of the maximum value of the array
     */
    double max_val = DBL_MIN;
    srand(32480842);
    // Find maximum of the function
    #pragma acc parallel loop reduction(max:max_val) present(array[0:size]) 
#pragma omp target teams loop reduction(max:max_val) map(present,\
            alloc:array[0:size])
    for (size_t i=0; i<size; ++i)
        if (array[i] > max_val) max_val = array[i];
    #pragma acc update self(array[0:size])
#pragma omp target update from(array[0:size])

    // Add noise
    for (size_t i=0; i<size; ++i)
        array[i] += 2.0*max_noise*max_val*((double)rand()/RAND_MAX-0.5);
    #pragma acc update device(array[0:size])
#pragma omp target update to(array[0:size])
}

double integral(double* array, size_t size, double start, double end)
{
    // Compute the integral
    double step = (end-start)/(double)size;
    double sum = 0.;
    #pragma acc parallel loop reduction(+:sum) present(array[0:size])
#pragma omp target teams loop reduction(+:sum) map(present,alloc:array[0:size])
    for (size_t i=0; i<size-1; ++i)
        sum += array[i] + array[i+1];
    return sum*0.5*step;
}

int main(void)
{
    size_t size = 100000000;
    double* array = init(size);

    // Gaussian parameters
    double variance = 0.1;
    double start = -5.0;
    double end = 5.0;
    gaussian(array, size, start, end, variance);
    double gauss_integral = integral(array, size, start, end);
    printf("Gaussian integral on [%f,%f] is %f\n", start, end, gauss_integral);

    // Make some noise!
    double max_noise = 0.1;
    add_noise(array, size, max_noise);
    double noised_integral = integral(array, size, start, end);
    #pragma acc exit data delete(array[0:size])
#pragma omp target exit data map(delete:array[0:size])
    free(array);
    printf("Noised Gaussian integral on [%f,%f] is %f\n", start, end, noised_integral);
    printf("The integral difference between the gaussian and the noised gaussian is: %10.4e\n", gauss_integral - noised_integral); 
    return 0;
}

// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report Data_Management_unstructured_update_device_solution.c
