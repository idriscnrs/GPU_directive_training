! you should add ` --option "-cpp" ` as argument to the idrrun command
program multigpu
    use ISO_FORTRAN_ENV, only : INT32
    use mpi
    use openacc
    implicit none
    integer(kind=INT32), dimension(100) :: a
    integer                             :: comm_size, my_rank, code, i
    integer                             :: num_gpus, my_gpu
    integer(kind=acc_device_kind)       :: device_type

    ! Useful for OpenMPI and GPU DIRECT
    call initialisation_openacc()

    ! MPI stuff
    call MPI_Init(code)
    call MPI_Comm_size(MPI_COMM_WORLD, comm_size, code)
    call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, code)

    ! OpenACC stuff
    #ifdef _OPENACC
    device_type = acc_get_device_type()
    num_gpus = acc_get_num_devices(device_type)
    my_gpu   = mod(my_rank,num_gpus)
    call acc_set_device_num(my_gpu, device_type)
    my_gpu   = acc_get_device_num(device_type)   
    ! Alternatively you can set the GPU number with #pragma acc set device_num(my_gpu)

    !$acc parallel loop
    do i = 1, 100
        a(i) = i
    enddo   
    #endif
    write(0,"(a13,i2,a17,i2,a8,i2,a10,i2)") "Here is rank ",my_rank,": I am using GPU ",my_gpu, & 
                                            " of type ",device_type,". a(42) = ",a(42)
    call MPI_Finalize(code)

    contains
        #ifdef _OPENACC
        subroutine initialisation_openacc
        use openacc
        
        type accel_info
            integer :: current_devices
            integer :: total_devices
        end type accel_info
       
        type(accel_info) :: info
        character(len=6) :: local_rank_env
        integer          :: local_rank_env_status, local_rank
        ! Initialisation of OpenACC
        !$acc init
 
       ! Recovery of the local rank of the process via the environment variable
       ! set by Slurm, as MPI_Comm_rank cannot be used here because this routine
       ! is used BEFORE the initialisation of MPI
       call get_environment_variable(name="SLURM_LOCALID", value=local_rank_env, status=local_rank_env_status)
       info%total_devices = acc_get_num_devices(acc_get_device_type())
       if (local_rank_env_status == 0) then
           read(local_rank_env, *) local_rank
           ! Definition of the GPU to be used via OpenACC
           call acc_set_device_num(local_rank, acc_get_device_type())
           info%current_devices = local_rank
       else
           print *, "Error : impossible to determine the local rank of the process"
           stop 1
       endif
       end subroutine initialisation_openacc
       #endif    

end program multigpu
