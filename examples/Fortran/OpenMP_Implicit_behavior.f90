program Implicit_behavior
    use iso_fortran_env, only : INT32, REAL64
    implicit none    
    
    real   (kind=REAL64), dimension(:)  , allocatable :: Array
    integer(kind=INT32 )                              :: nx, i, scalar

    nx = 10
    allocate(Array(nx))

    scalar = 1000
    !$omp target teams distribute parallel do simd
    do i = 1, nx
        Array(i) = scalar + i
    enddo

    print *, Array

    scalar = -1000
    !$omp target teams distribute parallel do simd
    do i = 1, nx
        Array(i) = scalar + i
    enddo

    print *, Array

    deallocate(Array)
end program Implicit_behavior
