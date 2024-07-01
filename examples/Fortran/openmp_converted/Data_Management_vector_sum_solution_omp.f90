program vector_sum
    use iso_fortran_env, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none

    integer(kind=INT32), parameter              :: system_size  = 10000
    integer(kind=INT32), dimension(system_size) :: a, b, c
    integer(kind=INT32)                         :: i

    !$acc parallel loop copyout(a(:), b(:)) 
!$omp target teams loop map(from:a(:),b(:))
    do i = 1, system_size
        a(i) = i
        b(i) = i * 2
    enddo

    !$acc parallel loop copyin(a(:), b(:)) copyout(c(:))
!$omp target teams loop map(to:a(:),b(:)) map(from:c(:))
    do i = 1, system_size
       c(i) = a(i) + b(i)
    enddo

    write(0,"(a22,i3)") "value at position 12: ", c(12)

end program vector_sum        

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Data_Management_vector_sum_solution.f90
