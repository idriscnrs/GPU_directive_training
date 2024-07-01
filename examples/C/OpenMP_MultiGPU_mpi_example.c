#include <stdio.h>
#include <mpi.h>
#include <openacc.h>
#include "../../examples/C/init_openacc.h"
int main(int argc, char** argv)
{
    // Useful for OpenMPI and GPU DIRECT
    initialisation_openacc();
    MPI_Init(&argc, &argv);
    
    // MPI Stuff
    int my_rank;
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int a[100];
    
    // OpenACC Stuff
    #ifdef _OPENACC
    acc_device_t device_type = acc_get_device_type();
    int num_gpus = acc_get_num_devices(device_type);
    int my_gpu = my_rank%num_gpus;
    acc_set_device_num(my_gpu, device_type);
    my_gpu = acc_get_device_num(device_type);
    // Alternatively you can set the GPU number with #pragma acc set device_num(my_gpu)
    
    #pragma acc parallel
    {
        #pragma acc loop
        for(int i = 0; i< 100; ++i)
            a[i] = i;
    }
    #endif
    printf("Here is rank %d: I am using GPU %d of type %d. a[42] = %d\n", my_rank, my_gpu, device_type, a[42]);
    MPI_Finalize();
    return 0;
}

