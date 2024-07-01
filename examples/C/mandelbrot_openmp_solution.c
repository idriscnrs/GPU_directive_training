#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#ifdef _OPENMP
   #include <omp.h>
#endif
#ifdef _OPENACC
   #include <openacc.h>
#endif
void output(unsigned char* picture, unsigned int width, unsigned int height)
{
   FILE* f = fopen("mandel.gray", "wb");
   fwrite(picture, sizeof(unsigned char), width*height, f);
   fclose(f);
}

#pragma acc routine seq
unsigned char mandelbrot_iterations(const float complex c)
{
    unsigned char max_iter = 255;
    unsigned char n = 0;
    float complex z = 0.0 + 0.0 * I;
    while (abs(z*z) <= 2. && n < max_iter)
    {
        z = z*z + c;
        ++n;
    }
    return n;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Please give width and height of the world.");
        return 1;
    }
    unsigned int width = (unsigned int) atoi(argv[1]);
    float step_w = 1./width;
    unsigned int height = (unsigned int) atoi(argv[2]);
    float step_h = 1./height;
    unsigned char* restrict picture = (unsigned char*) malloc(width*height*sizeof(unsigned char));
    // Here we set the bonds of the coordinates of the picture.
    const float min_re = -2;
    const float max_re = 1;
    const float min_im = -1;
    const float max_im = 1;

    struct timespec end, start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    int rank = 0;
    unsigned int first = 0;
    unsigned int last = height;
    int num_elements = width*height;
#pragma omp parallel private(first, last, rank) shared(picture) firstprivate(height, width, min_re, max_re, min_im, max_im, step_h, step_w, num_elements) default(none)
{
#ifdef _OPENMP
    rank = omp_get_thread_num();
    int num_threads = omp_get_num_threads();
    first = rank * (height/num_threads);
    last  = (rank + 1) * (height/num_threads);
    num_elements = width*height/num_threads;
#pragma omp master
{
    printf("Using OpenMP\n");
}
    printf("I am rank %2d and my range is [%5d, %5d[ ie %10d elements\n", rank, first, last, num_elements);
#endif
    
#ifdef _OPENACC
    acc_device_t type = acc_get_device_type();
    int num_gpu = acc_get_num_devices(type);
    acc_set_device_num(rank%num_gpu, type);
    printf("I am rank %2d. I am using GPU %d\n", rank, acc_get_device_num(type));
#endif

#pragma acc parallel copyout(picture[first*width:num_elements])
    {
    #pragma acc loop 
    for (unsigned int i=first; i<last; ++i)
        for (unsigned int j=0; j<width; ++j)
        {
            float complex c;
            c = min_re + j * step_w * (max_re - min_re) + \
                I * (min_im + ( i * step_h) * (max_im - min_im));
            picture[width*i+j] = (unsigned char) 255 - mandelbrot_iterations(c);
        }
    }
}
    // Measure time
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    unsigned long int delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("The time to generate the mandelbrot picture was %10.5e s\n", delta_us/1.e6);
    output(picture, width, height);
}
