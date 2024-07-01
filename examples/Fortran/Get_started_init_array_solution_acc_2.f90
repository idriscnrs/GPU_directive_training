program array_initialisation
    use iso_fortran_env, only : INT32, REAL64
    use openacc
    implicit none

    integer(kind=INT32), parameter :: system_size = 100000
    integer(kind=INT32)            :: array(system_size)
    integer(kind=INT32)            :: i

    !$acc parallel

    !$acc loop
    do i = 1, system_size
        array(i) = 2*i
    enddo

    !$acc end parallel

    write(0,"(a22,i3)") "value at position 21: ", array(21)    
end program array_initialisation
