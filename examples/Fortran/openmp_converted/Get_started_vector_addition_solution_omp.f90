program vector_addition
    use iso_fortran_env, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none

    integer(kind=INT32 ), parameter              :: system_size = 1e5
    real   (kind=REAL64), dimension(system_size) :: s, c, array_sum
    real   (kind=REAL64)                         :: fortran_pi
    integer(kind=INT32 )                         :: i

    fortran_pi = acos(-1.0_real64)

    !$acc data create(s(:), c(:)) copyout(array_sum(:))
!$omp target data map(from:array_sum(:)) map(alloc:s(:),c(:))

    !$acc parallel
!$omp target teams
    !$acc loop
!$omp loop
    do i = 1, system_size
        s(i) = sin(i*fortran_pi/system_size) * sin(i*fortran_pi/system_size)
        c(i) = cos(i*fortran_pi/system_size) * cos(i*fortran_pi/system_size)
    enddo
    !$acc end parallel
!$omp end target teams

    !$acc parallel
!$omp target teams
    !$acc loop
!$omp loop
    do i = 1, system_size - 1 
        array_sum(i) = s(i) + c(system_size - i)
    enddo
    !$acc end parallel
!$omp end target teams

    !$acc end data
!$omp end target data
    
    write(0,"(a10,f10.8)") "sum(42) = ",array_sum(42)
end program vector_addition 

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Get_started_vector_addition_solution.f90
