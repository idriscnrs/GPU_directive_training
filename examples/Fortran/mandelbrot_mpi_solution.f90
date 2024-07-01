! you should add ` --option "-cpp" ` as argument to the idrrun command
program mandelbrot_mpi
    use MPI
    #ifdef _OPENACC
    use openacc
    #endif
    implicit none
    type accel_info
        integer :: current_devices
        integer :: total_devices
    end type accel_info                                              
    type(accel_info)              :: gpu_info
    real, parameter               :: min_re = -2.0, max_re = 1.0
    real, parameter               :: min_im = -1.0, max_im = 1.0
    integer                       :: first, last, width, height
    integer                       :: num_elements
    real                          :: step_w, step_h
    integer                       :: numarg, i, length, j, first_elem,last_elem
    integer                       :: rest_eucli,local_height
    integer                       :: rank, nb_procs, code
    character(len=:), allocatable :: arg1, arg2
    integer (kind=1), allocatable :: picture(:)
    real                          :: x, y

    #ifdef _OPENACC
    gpu_info = initialisation_openacc()
    #endif   

    numarg = command_argument_count()
    if (numarg .ne. 2) then
        width = 4000
        height = 4000
    else
        call get_command_argument(1,LENGTH=length)
        allocate(character(len=length) :: arg1)
        call get_command_argument(1,VALUE=arg1)
        read(arg1,'(i10)') width
        call get_command_argument(2,LENGTH=length)
        allocate(character(len=length) :: arg2)
        call get_command_argument(2,VALUE=arg2)    
        read(arg2,'(i10)') height
    endif
    step_w = 1.0 / real(width)                                                    
    step_h = 1.0 / real(height)

    call mpi_init(code)
    call mpi_comm_rank(MPI_COMM_WORLD,rank,code)
    call mpi_comm_size(MPI_COMM_WORLD,nb_procs,code)

    local_height = height / nb_procs
    first = 0
    last  = local_height
    rest_eucli = mod(height,nb_procs)

    if ((rank .eq. 0) .and. (rank .lt. rest_eucli)) last = last + 1

    if (rank .gt. 0) then
        do i = 1, rank
            first = first + local_height
            last  = last  + local_height
                if (rank .lt. rest_eucli) then
                    first = first + 1
                    last  = last  + 1
                endif
        enddo
    endif

    if (rank .lt. rest_eucli) local_height = local_height + 1
    num_elements = local_height * width

    write(unit=*,fmt="(a9,i3,a18,i8,a3,i8,a5,i10,a9)") "I am rank",rank, &
    " and my range is [",first," ,",last,"[ ie ",num_elements," elements"

    allocate(picture(first*width:last*width))
    !$acc data copyout(picture(first*width:last*width))
    !$acc parallel loop private(x,y)
    do i=first,last-1
        !$acc loop
        do j=0,width-1
            x =  min_re + j * step_w * (max_re - min_re)
            y =  min_im + i * step_h * (max_im - min_im)
            picture(i*width+j) = mandelbrot_iterations(x,y)
        enddo
    enddo
    !$acc end data
    call output()
    deallocate(picture)

    call mpi_finalize(code)

    contains
        #ifdef _OPENACC
        type(accel_info) function initialisation_openacc
           ! use openacc
            type(accel_info) :: info
            character(len=6) :: local_rank_env
            integer          :: local_rank_env_status, local_rank
            ! Initialisation of OpenACC
            !$acc init
   
            ! Recovery of the local rank of the process via the environment variable
            ! set by Slurm, as MPI_Comm_rank cannot be used here because this routine
            ! is used BEFORE the initialisation of MPI
            call get_environment_variable(name="SLURM_LOCALID", value=local_rank_env, &
                                          status=local_rank_env_status)
            initialisation_openacc%total_devices   = acc_get_num_devices(acc_get_device_type())
            initialisation_openacc%current_devices = -1
            if (local_rank_env_status == 0) then
                read(local_rank_env, *) local_rank
                ! Definition of the GPU to be used via OpenACC
                call acc_set_device_num(local_rank, acc_get_device_type())
                initialisation_openacc%current_devices = local_rank
            else
                print *, "Error : impossible to determine the local rank of the process"
                stop 1
            endif
        end function initialisation_openacc
        #endif
        subroutine output
            integer                         :: fh
            integer(kind=MPI_OFFSET_KIND)   :: woffset

            woffset=first*width
            call MPI_File_open(MPI_COMM_WORLD,"mandel.gray",MPI_MODE_WRONLY+MPI_MODE_CREATE,MPI_INFO_NULL,fh,code)
            call MPI_File_write_at(fh,woffset,picture,num_elements,MPI_INTEGER1,MPI_STATUS_IGNORE,code);
            call MPI_File_close(fh,code)
        end subroutine output
        integer(kind=1) function mandelbrot_iterations(x,y)
            !$acc routine seq
            integer, parameter              :: max_iter = 127
            real, intent(in)                :: x,y
            real                            :: z1,z2,z1_old,z2_old

            z1 = 0.0
            z2 = 0.0
            mandelbrot_iterations = 0
            do while (((z1*z1+z2*z2) .le. 4) .and. (mandelbrot_iterations .lt. max_iter))
                z1_old = z1
                z2_old = z2
                z1 = z1_old*z1_old - z2_old*z2_old  + x
                z2 = 2.0*z1_old*z2_old + y
                mandelbrot_iterations = mandelbrot_iterations + 1
            enddo
        end function mandelbrot_iterations      
end program mandelbrot_mpi
