program wrong_usage
    use iso_fortran_env, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none
        
    integer(kind=INT32 ), parameter              :: system_size = 10000
    real   (kind=real64), dimension(system_size) :: a, b, c
    integer(kind=INT32 )                         :: i

!  Structured data region
    !$acc data create(a, b) copyout(c)
!$omp target data map(from:c) map(alloc:a,b)

        !$acc parallel loop present(a(:), b(:))
!$omp target teams loop
        do i = 1, system_size
            a(i) = i
            b(i) = i*2
        enddo

        ! We update an element of the array on the CPU
        !$acc update host (a(:))
!$omp target update from(a(:))
        a(12) = 42
        !$acc update device (a(:))
!$omp target update to(a(:))

        !$acc parallel loop present(b(:), c(:)) copyin(a(:))
!$omp target teams loop 
        do i = 1, system_size
            c(i) = a(i) + b(i)
        enddo
    !$acc end data
!$omp end target data
    write(0,"(a22,f10.5)") "value at position 12: ", c(12)
end program wrong_usage

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Data_Management_wrong_example.f90
