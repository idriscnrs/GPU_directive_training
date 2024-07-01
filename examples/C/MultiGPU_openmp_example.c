#include <stdio.h>
#include <openacc.h>
#include <omp.h>
int main(int argc,char** argv)
{
    #pragma omp parallel 
    {
        int my_rank = omp_get_thread_num();
        // OpenACC Stuff
        #ifdef _OPENACC
        acc_device_t dev_type = acc_get_device_type();
        int num_gpus = acc_get_num_devices(dev_type);
        int my_gpu = my_rank%num_gpus;
        acc_set_device_num(my_gpu, dev_type);
        // We check what GPU is really in use
        my_gpu = acc_get_device_num(dev_type);
        // Alternatively you can set the GPU number with #pragma acc set device_num(my_gpu)
        printf("Here is thread %d: I am using GPU %d of type %d.\n", my_rank, my_gpu, dev_type);
        #endif  
    }
}

