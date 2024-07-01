! you should add ` --option "-cpp" ` as argument to the idrrun command
program mandelbrot
    #ifdef _OPENMP
    use OMP_LIB
    #endif
    #ifdef _OPENACC 
    use openacc
    #endif
    implicit none
    integer, parameter               :: max_iter = 127
    real, parameter                  :: min_re = -2.0
    real, parameter                  :: max_re = 1.0
    real, parameter                  :: min_im = -1.0
    real, parameter                  :: max_im = 1.0
    integer                          :: rank
    integer                          :: first,last,width,height
    integer                          :: num_elements,num_threads,num_gpu
    real                             :: step_w,step_h
    character(len=:), allocatable    :: arg1,arg2
    integer (kind=1), allocatable    :: picture(:)
    real                             :: x,y
    integer                          :: len1,len2
    integer                          :: i,j,first_elem,last_elem
    #ifdef _OPENACC
    integer(acc_device_kind)         :: type_d
    #endif

    call GET_COMMAND_ARGUMENT(1,LENGTH=len1)
    allocate(character(len=len1) :: arg1)
    call GET_COMMAND_ARGUMENT(1,VALUE=arg1)
    call GET_COMMAND_ARGUMENT(2,LENGTH=len2)
    allocate(character(len=len2) :: arg2)
    call GET_COMMAND_ARGUMENT(2,VALUE=arg2)
    read(arg1,'(i10)') width
    read(arg2,'(i10)') height
    step_w = 1.0 / width
    step_h = 1.0 / height

    deallocate(arg1,arg2)

    allocate(picture(0:width*height-1))


    !$OMP parallel private(first, last, rank, num_threads, num_elements, num_gpu, x, y, i, j, last_elem, first_elem) shared(picture) firstprivate(height, width, step_h, step_w) default(none)

    #ifdef _OPENMP
     rank         = OMP_GET_THREAD_NUM()
     num_threads  = OMP_GET_NUM_THREADS()
     first        =  rank * (height/num_threads)
     last         = (rank + 1) * (height/num_threads) -1
     num_elements = width*height/num_threads
    #endif

    print *, "Using OpenMP"

    write(unit=*,fmt="(a9,i3,a18,i8,a3,i8,a5,i10,a9)") "I am rank",rank, &
          " and my range is [",first," ,",last,"[ ie ",num_elements," elements"

    #ifdef _OPENACC
     type_d  = acc_get_device_type()
     num_gpu = acc_get_num_devices(type_d)
     call acc_set_device_num(mod(rank,num_gpu), type_d)
     first_elem   = first*width
     last_elem    = first_elem + num_elements-1
     print *, "I am rank",rank,". I am using GPU #", &
              acc_get_device_num(type_d),first_elem,last_elem
    #endif

    do i = first, last
         do j = 0, width-1
             x =  min_re + j * step_w * (max_re - min_re)
             y =  min_im + i * step_h * (max_im - min_im)
             picture(i*width+j) = mandelbrot_iterations(x,y)
         enddo
    enddo

    !$OMP end parallel


    open(unit=10, FILE="mandel.gray", ACCESS="stream", FORM="unformatted")
        write(unit=10,pos=1) picture 
    close(10)

    deallocate(picture)
    contains
    integer(kind=1) function mandelbrot_iterations(x,y)
        !$acc routine seq
        real, intent(in) :: x,y
        real             :: z1,z2,z1_old,z2_old
        z1 = 0.0
        z2 = 0.0
        mandelbrot_iterations = 0
        do while (((z1*z1+z2*z2) .le. 4) .and. (mandelbrot_iterations .lt. max_iter))
            z1_old = z1
            z2_old = z2
            z1 = z1_old*z1_old - z2_old*z2_old  + x
            z2 = 2.0*z1_old*z2_old      + y
            mandelbrot_iterations = mandelbrot_iterations + 1
        enddo
    end function mandelbrot_iterations
end program mandelbrot
