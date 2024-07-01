#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <mpi.h>
#include <omp.h>
void output(unsigned char* picture, unsigned int start, unsigned int num_elements)
{
   MPI_File     fh;
   MPI_Offset   woffset=start;

   if (MPI_File_open(MPI_COMM_WORLD,"mandel.gray",MPI_MODE_WRONLY+MPI_MODE_CREATE,MPI_INFO_NULL,&fh) != MPI_SUCCESS)
   {
        fprintf(stderr,"ERROR in creating output file\n");
        MPI_Abort(MPI_COMM_WORLD,1);
   }

   MPI_File_write_at(fh,woffset,picture,num_elements,MPI_UNSIGNED_CHAR,MPI_STATUS_IGNORE);

   MPI_File_close(&fh);
} 

#pragma omp declare target
unsigned char mandelbrot_iterations(const float complex c)
{
    unsigned char max_iter = 255;
    unsigned char n = 0;
    float complex z = 0.0 + 0.0 * I;
    while (abs(z*z) <= 2 && n < max_iter)
    {
        z = z*z + c;
        ++n;
    }
    return n;
}
#pragma omp end declare target
int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    unsigned int width = (unsigned int) atoi(argv[1]);
    float step_w = 1./width;
    unsigned int height = (unsigned int) atoi(argv[2]);
    float step_h = 1./height;

    const float min_re = -2.;
    const float max_re = 1.;
    const float min_im = -1.;
    const float max_im = 1.;

    struct timespec end, start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    int i;
    int rank;
    int nb_procs;
    int total_devices;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_procs);

    unsigned int local_height = height / nb_procs;
    unsigned int first = 0;
    unsigned int last = local_height;
    unsigned int rest_eucli = height % nb_procs;

    if ((rank==0) && (rank < rest_eucli))
          ++last;

    for (i=1; i <= rank; ++i)
    {
      first += local_height;
      last  += local_height;
      if (rank < rest_eucli)
          {
              ++first;
              ++last;
          }
    }

    if (rank < rest_eucli) 
        ++local_height;

    unsigned int num_elements = width*local_height;
    if (rank == 0) printf("Using MPI\n");
    total_devices = omp_get_num_devices();
    printf("I am rank %2d and my range is [%5d, %5d[ ie %10d elements. Runing on %d GPUs.\n", rank, first, last, num_elements, total_devices);
    unsigned char* restrict picture = (unsigned char*) malloc(num_elements*sizeof(unsigned char));
#pragma omp target data map(tofrom:picture[0:num_elements]) device(rank)
{    
#pragma omp target teams distribute parallel for simd collapse(2) device(rank)
    for (unsigned int i=0; i<local_height; ++i)
        for (unsigned int j=0; j<width; ++j)
        {
            float complex c;
            c = min_re + j*step_w * (max_re - min_re) + \
                I * (min_im +  ((i+first) * step_h) * (max_im - min_im));
            picture[width*i+j] = (unsigned char) (255-rank*(255/nb_procs)) - mandelbrot_iterations(c);
        }
}
    output(picture, first*width, num_elements); 
    MPI_Finalize();

    // Measure time
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    unsigned long int delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("The time to generate the mandelbrot picture was %lu us\n", delta_us);
    return EXIT_SUCCESS;
}
