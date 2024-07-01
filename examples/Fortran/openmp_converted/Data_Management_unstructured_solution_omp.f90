program allocate_array_separately
    use iso_fortran_env, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none

    real   (kind=REAL64), dimension(:), allocatable :: array
    integer(kind=INT32 )                            :: system_size
    integer(kind=INT32 )                            :: i
 
    system_size = 100000

    call init(array, system_size)

    !$acc parallel loop present(array(:))
!$omp target teams loop
    do i = 1, system_size
        array(i) = dble(i)
    enddo

    !$acc exit data copyout(array(:))
!$omp target exit data map(from:array(:))
    write(0,*) array(42)

    deallocate(array)

    contains
     subroutine init(array, system_size)            
     
     real   (kind=REAL64), dimension(:), allocatable, intent(inout) :: array
     integer(kind=INT32 ), intent(in)                               :: system_size

     allocate(array(system_size))
     !$acc enter data create(array(1:system_size))
!$omp target enter data map(alloc:array(1:system_size))

     end subroutine init
end program allocate_array_separately

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Data_Management_unstructured_solution.f90
