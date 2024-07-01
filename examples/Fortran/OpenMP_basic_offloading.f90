program basic_offloading
    use iso_fortran_env, only : real64
    implicit none    
    
    integer                                      :: i, sys_size
    real(kind=real64), allocatable, dimension(:) :: array

    sys_size = 100000000

    allocate(array(sys_size))

    !$omp target teams distribute parallel do simd
    do i=1, sys_size
        array(i) = real(i)
    enddo
    
    print *, "array(42) = ", array(42)
end program basic_offloading
