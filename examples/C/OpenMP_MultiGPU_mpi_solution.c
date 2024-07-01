#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openmp.h>
#include <math.h>
#include "../../examples/init_omp_target.h"

int main(int argc, char** argv)
{
    initialisation_openacc();
    MPI_Init(&argc, &argv);
    fflush(stdout);
    double start;
    double end;
    
    int size = 200000000/8;
    
    double* send_buffer = (double*)malloc(size*sizeof(double));
    double* receive_buffer = (double*)malloc(size*sizeof(double));
    #pragma omp targer enter data map(alloc: send_buffer[:size], receive_buffer[:size])
    // MPI Stuff
    int my_rank;
    int comm_size;
    int reps = 5;
    double data_volume = (double)reps*(double)size*sizeof(double)*pow(1024,-3.0);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Status status;
    
    // OpenMP target Stuff
    int num_gpus = omp_get_num_devices();
    int my_gpu = my_rank%num_gpus;
    acc_set_device_num(my_gpu, device_type); 
    for (int i = 0; i<comm_size; ++i)
    {
        for (int j=0; j < comm_size; ++j)
        {
            if (my_rank == i && i != j)
            {
                start = MPI_Wtime();
                #pragma omp target data use_device_ptr(send_buffer)
                {
                    for (int k = 0 ; k < reps; ++k)
                        MPI_Ssend(send_buffer, size, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
                }
            }
            if (my_rank == j && i != j)
            {
                #pragma omp target data use_device_ptr(receive_buffer)
                {
                    for (int k = 0 ; k < reps; ++k)
                        MPI_Recv(receive_buffer, size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
                }
            }
            if (my_rank == i && i != j)
            {
                end = MPI_Wtime();
                printf("bandwidth %d->%d: %10.5f GB/s\n", i, j, data_volume/(end-start));
            }
        }
    }
    #pragma omp targer exit data map(delete: send_buffer[:size], receive_buffer[:size])
    MPI_Finalize();
    return 0;
}

