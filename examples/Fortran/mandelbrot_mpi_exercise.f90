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
    ! add initialisation here as subroutine call or function assignation to a type(accel_info) variable
    #endif   

    numarg = command_argument_count()
    if (numarg .ne. 2) then
        write(0,*) "Error, you should provide 2 arguments of integer kind : width and length"
        stop
    endif
    call get_command_argument(1,LENGTH=length)
    allocate(character(len=length) :: arg1)
    call get_command_argument(1,VALUE=arg1)
    read(arg1,'(i10)') width
    call get_command_argument(2,LENGTH=length)
    allocate(character(len=length) :: arg2)
    call get_command_argument(2,VALUE=arg2)    
    read(arg2,'(i10)') height
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
    do i=first,last-1
        do j=0,width-1
            x =  min_re + j * step_w * (max_re - min_re)
            y =  min_im + i * step_h * (max_im - min_im)
            picture(i*width+j) = mandelbrot_iterations(x,y)
        enddo
    enddo
    call output()
    deallocate(picture)

    call mpi_finalize(code)

    contains
        #ifdef _OPENACC
        ! implement function or subroutine here :
        !           function :
        !type(accel_info) function initialisation_openacc
        !end function initialisation_openacc
        !
        !           subroutine :
        !subroutine initialisation_openacc()
        !end subroutine initialisation_openacc
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
