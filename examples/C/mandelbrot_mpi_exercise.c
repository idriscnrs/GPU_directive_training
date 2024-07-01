// you can use ` --option "-DMULTIGPU" `  to print the device info after filling the openacc initialisation
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _OPENACC
  #include <openacc.h>
#endif
#include <complex.h>
#include <mpi.h>
// add openacc initialisation
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

#pragma acc routine seq
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
int main(int argc, char** argv)
{
    #ifdef _OPENACC
    // add initilisation openacc
    #endif   
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
    #if defined(_OPENACC) && defined(MULTIGPU)
    printf("I am rank %2d and my range is [%5d, %5d[ ie %10d elements. I use GPU %d over %d devices.\n", rank, first, last, num_elements,info.current_device, info.total_devices);
    #else
    printf("I am rank %2d and my range is [%5d, %5d[ ie %10d elements.", rank, first, last, num_elements);
    #endif

    unsigned char* restrict picture = (unsigned char*) malloc(num_elements*sizeof(unsigned char));

#pragma acc data copyout(picture[0:num_elements])
{
#pragma acc parallel loop
    for (unsigned int i=0; i<local_height; ++i)
        for (unsigned int j=0; j<width; ++j)
        {
            float complex c;
            c = min_re + j*step_w * (max_re - min_re) + \
                I * (min_im +  ((i+first) * step_h) * (max_im - min_im));
            picture[width*i+j] = (unsigned char)255 - mandelbrot_iterations(c);
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

