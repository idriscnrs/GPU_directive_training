program manual_build
    use iso_fortran_env, only : INT32, REAL64
    implicit none

    real   (kind=REAL64), dimension(:), allocatable :: A, B
    real   (kind=REAL64)                            :: summation, fortran_pi
    integer(kind=INT32 )                            :: system_size
    integer(kind=INT32 )                            :: i

    fortran_pi  = acos(-1.0_real64)
    summation   = 0.0_real64
    system_size = 1e9
    allocate(A(system_size), B(system_size))

    !$acc data create(A(:), B(:))
!$omp target data map(alloc:a(:),b(:))
    !$acc parallel loop present(A(:), B(:))
!$omp target teams loop
    do i = 1, system_size
        A(i) = sin(i*fortran_pi/system_size) * sin(i*fortran_pi/system_size)
        B(i) = cos(i*fortran_pi/system_size) * cos(i*fortran_pi/system_size)
    enddo

    call inplace_sum(A, B, system_size)

    !$acc parallel loop present(A(:), B(:)) reduction(+:summation)
!$omp target teams loop reduction(+:summation)
    do i = 1, system_size
        summation = summation + A(i)
    enddo    
    !$acc end data
!$omp end target data
    write(0,"(a29,f10.8)") "This should be close to 1.0: ", summation/dble(system_size)
    deallocate(A, B)
    contains
        subroutine inplace_sum(A, B, n)
        real   (kind=REAL64), dimension(:), intent(inout) :: A, B
        integer(kind=INT32 ), intent(in)                  :: n
        !$acc parallel loop present(A(:), B(:))
!$omp target teams loop
        do i = 1, n
            A(i) = A(i) + B(i)
        enddo
        end subroutine inplace_sum
end program manual_build

