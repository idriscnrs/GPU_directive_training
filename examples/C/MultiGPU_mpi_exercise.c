#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openacc.h>
#include <math.h>
#include "../../examples/C/init_openacc.h"
int main(int argc, char** argv)
{
    initialisation_openacc();
    MPI_Init(&argc, &argv);
    fflush(stdout);
    double start;
    double end;
    
    int size = 2e8/8;
    
    double* send_buffer = (double*)malloc(size*sizeof(double));
    double* receive_buffer = (double*)malloc(size*sizeof(double));
    // MPI Stuff
    int my_rank;
    int comm_size;
    int reps = 5;
    double data_volume = (double)reps*(double)size*sizeof(double)*pow(1024,-3.0);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Status status;
    
    // OpenACC Stuff
    acc_device_t device_type = acc_get_device_type();
    int num_gpus = acc_get_num_devices(device_type);
    int my_gpu = my_rank%num_gpus;
    acc_set_device_num(my_gpu, device_type); 
    for (int i = 0; i<comm_size; ++i)
    {
        for (int j=0; j < comm_size; ++j)
        {
            if (my_rank == i && i != j)
            {
                start = MPI_Wtime();
                for (int k = 0 ; k < reps; ++k)
                    MPI_Ssend(send_buffer, size, MPI_DOUBLE, j, 0, MPI_COMM_WORLD);
            }
            if (my_rank == j && i != j)
            {
                for (int k = 0 ; k < reps; ++k)
                    MPI_Recv(receive_buffer, size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
            }
            if (my_rank == i && i != j)
            {
                end = MPI_Wtime();
                printf("bandwidth %d->%d: %10.5f GB/s\n", i, j, data_volume/(end-start));
            }
        }
    }
    MPI_Finalize();
    return 0;
}

