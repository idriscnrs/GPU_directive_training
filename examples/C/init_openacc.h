#ifdef _OPENACC
#include <stdio.h>
#include <openacc.h>
#include <stdlib.h>
typedef struct{
    int current_device;
    int total_devices;
}acc_info;

acc_info initialisation_openacc()
{
    char* local_rank_env;
    int local_rank;
    int total_devices;
    acc_info info;
 
    /* Initialisation of OpenACC */
    #pragma acc init
 
    /* Recovery of the local rank of the process via the environment variable
       set by Slurm, as MPI_Comm_rank cannot be used here because this routine
       is used BEFORE the initialisation of MPI*/
 
    local_rank_env = getenv("SLURM_LOCALID");
    total_devices = acc_get_num_devices(acc_get_device_type());
    info.total_devices = total_devices;
 
    if (local_rank_env) {
        local_rank = atoi(local_rank_env)%total_devices;
        /* Define the GPU to use via OpenACC */
        acc_set_device_num(local_rank, acc_get_device_type());
        info.current_device = local_rank;
    } else {
        printf("Error : impossible to determine the local rank of the process\n");
        exit(1);
    }
    return info;
}
#endif
// Leave one blank line below

