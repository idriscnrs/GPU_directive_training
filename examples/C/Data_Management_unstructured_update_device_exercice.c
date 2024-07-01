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
    for (size_t i=0; i<size; ++i)
        array[i] = 0.;
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
        for (double g=start; g<=end; g += range/num_gauss)
            #pragma acc loop private(position)
            for (size_t i=0; i<size; ++i)
            {
                position = start + i*step;
                array[i] += norm * exp(-0.5*(position-g)*(position-g)/(variance*variance));
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
    for (size_t i=0; i<size; ++i)
        if (array[i] > max_val) max_val = array[i];

    // Add noise
    for (size_t i=0; i<size; ++i)
        array[i] += 2.0*max_noise*max_val*((double)rand()/RAND_MAX-0.5);
}

double integral(double* array, size_t size, double start, double end)
{
    /**
     * Compute the integral
     */
    double step = (end-start)/(double)size;
    double sum = 0.;
    for (size_t i=0; i<size-1; ++i)
    {
        sum += array[i] + array[i+1];
    }
    return sum*0.5*step;
}

int main(void)
{
    size_t size = 20000;
    double* array = init(size);

    // Gaussian parameters
    double exp_val = 0.0;
    double variance = 1.0;
    double start = -5.0;
    double end = 5.0;
    gaussian(array, size, start, end, variance);
    double gauss_integral = integral(array, size, start, end);
    printf("Gaussian integral on [%f,%f] is %f\n", start, end, gauss_integral);

    // Make some noise!
    double max_noise = 0.01;
    add_noise(array, size, max_noise);
    double noised_integral = integral(array, size, start, end);
    free(array);
    printf("Noised Gaussian integral on [%f,%f] is %f\n", start, end, noised_integral);
    printf("The integral difference between the gaussian and the noised gaussian is: %10.4e\n", gauss_integral - noised_integral); 
    return 0;
}
