! you should add ` --option "-cpp" ` as argument to the idrrun command
program MultiGPU_solution
    use ISO_FORTRAN_ENV, only : INT32, REAL64
    use mpi
    use openacc
    implicit none
    real   (kind=REAL64), dimension(:), allocatable :: send_buffer, receive_buffer
    real   (kind=REAL64)                            :: start, finish , data_volume   
    integer(kind=INT32 ), parameter                 :: system_size = 2e8/8
    integer                                         :: comm_size, my_rank, code, reps, i, j, k
    integer                                         :: num_gpus, my_gpu
    integer(kind=acc_device_kind)                   :: device_type
    integer, dimension(MPI_STATUS_SIZE)             :: mpi_stat

    ! Useful for OpenMPI and GPU DIRECT
    call initialisation_openacc()

    ! MPI stuff
    reps = 5
    data_volume = dble(reps*system_size)*8*1024_real64**(-3.0)

    call MPI_Init(code)
    call MPI_Comm_size(MPI_COMM_WORLD, comm_size, code)
    call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, code)
    allocate(send_buffer(system_size), receive_buffer(system_size))
    !$omp target enter data map(alloc: send_buffer(1:system_size), receive_buffer(1:system_size))

    ! OpenMP target stuff
    #ifdef _OPENACC
    device_type = acc_get_device_type()
    num_gpus = acc_get_num_devices(device_type)
    my_gpu   = mod(my_rank,num_gpus)
    call acc_set_device_num(my_gpu, device_type)
    #endif

    do j = 0, comm_size - 1
        do i = 0, comm_size - 1
            if ( (my_rank .eq. j) .and. (j .ne. i) ) then
                start = MPI_Wtime()
                !$omp target data use_device_ptr(send_buffer)
                do k = 1, reps
                    call MPI_Send(send_buffer,system_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, code)
                enddo
                !$omp end target data
            endif 
            if ( (my_rank .eq. i) .and. (i .ne. j) ) then
                !$omp target data use_device_ptr(send_buffer)
                do k = 1, reps
                    call MPI_Recv(receive_buffer, system_size, MPI_DOUBLE, j, 0, MPI_COMM_WORLD, mpi_stat, code)
                enddo
                !$omp end target data
            endif
            if ( (my_rank .eq. j) .and. (j .ne. i) ) then
                finish = MPI_Wtime()
                write(0,"(a11,i2,a2,i2,a2,f20.8,a5)") "bandwidth ",j,"->",i,": ",data_volume/(finish-start)," GB/s"
            endif
        enddo
    enddo
    !$omp target exit data map(delete: send_buffer, receive_buffer)
    deallocate(send_buffer, receive_buffer)
    
    call MPI_Finalize(code)

    contains
        #ifdef _OPENACC
        subroutine initialisation_openacc
            use openacc
            implicit none
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

end program MultiGPU_solution
