program allocate_array_separately
    use iso_fortran_env, only : INT32, REAL64
    use openacc
    implicit none

    real   (kind=REAL64), dimension(:), allocatable :: array
    integer(kind=INT32 )                            :: system_size
    integer(kind=INT32 )                            :: i
 
    system_size = 100000

    call init(array, system_size)

    do i = 1, system_size
        array(i) = dble(i)
    enddo

    write(0,*) array(42)

    deallocate(array)

    contains
     subroutine init(array, system_size)            
     
     real   (kind=REAL64), dimension(:), allocatable, intent(inout) :: array
     integer(kind=INT32 ), intent(in)                               :: system_size

     allocate(array(system_size))

     end subroutine init
end program allocate_array_separately
