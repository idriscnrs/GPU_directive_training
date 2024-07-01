! you should add ` --option "-cpp" ` as argument to the idrrun command
program MultiGPU_openmp
    use ISO_FORTRAN_ENV, only : INT32
    use OMP_LIB
    use openacc
    implicit none    
    integer(kind=INT32)           :: my_rank
    integer                       :: num_gpus, my_gpu
    integer(kind=acc_device_kind) :: device_type

    !$omp parallel private(my_rank, my_gpu, device_type)
        my_rank = omp_get_thread_num()
        ! OpenACC Stuff
        #ifdef _OPENACC
        device_type = acc_get_device_type()
        num_gpus = acc_get_num_devices(device_type)
        my_gpu   = mod(my_rank,num_gpus)
        call acc_set_device_num(my_gpu, device_type)
        ! We check what GPU is really in use
        my_gpu = acc_get_device_num(device_type)
        ! Alternatively you can set the GPU number with #pragma acc set device_num(my_gpu)
        write(0,"(a14,i2,a17,i2,a9,i2)") "Here is thread ",my_rank," : I am using GPU ",my_gpu," of type ",device_type
        #endif  
    !$omp end parallel
end program MultiGPU_openmp

