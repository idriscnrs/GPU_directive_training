program array_initialisation
    use iso_fortran_env, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none

    integer(kind=INT32), parameter :: system_size = 100000
    integer(kind=INT32)            :: array(system_size)
    integer(kind=INT32)            :: i

    !$acc parallel
!$omp target teams

    !$acc loop
!$omp loop
    do i = 1, system_size
        array(i) = 2*i
    enddo

    !$acc end parallel
!$omp end target teams

    write(0,"(a22,i3)") "value at position 21: ", array(21)    
end program array_initialisation

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Get_started_init_array_solution_acc_2.f90
