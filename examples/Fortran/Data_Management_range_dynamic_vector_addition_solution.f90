program range_dynamic
    use iso_fortran_env, only : INT32, REAL64
    implicit none
    integer(kind=INT32), dimension(:), allocatable :: a, b, c
    integer(kind=INT32)                            :: i
    integer(kind=INT32)                            :: system_size    

    system_size = 10000
    allocate(a(system_size), b(system_size), c(system_size))

    !$acc data create(a(:), b(:)) copyout(c(:))
    !$acc parallel loop present(a(:), b(:))
    do i = 1, system_size
        a(i) = i
        b(i) = i*2
    enddo

    !$acc parallel loop present(a(:), b(:), c(:))
    do i = 1, system_size
        c(i) = a(i) + b(i)
    enddo
    !$acc end data

    write(0,"(a21,i5)") "Value at position 12: ", c(12)
    deallocate(a, b, c)
end program
