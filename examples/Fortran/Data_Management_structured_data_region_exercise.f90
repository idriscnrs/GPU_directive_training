program vector_addition
    use iso_fortran_env, only : INT32
    use openacc
    implicit none

    integer(kind=INT32), parameter              :: system_size  = 10000
    integer(kind=INT32), dimension(system_size) :: a, b, c
    integer(kind=INT32)                         :: i

!   Structured data region

    !$acc parallel loop present(a(:), b(:))
    do i = 1, system_size
        a(i) = i
        b(i) = i * 2
    enddo

    !$acc parallel loop present(a(:), b(:), c(:))
    do i = 1, system_size
       c(i) = a(i) + b(i)
    enddo

!   End of structured data region

    write(0,"(a22,i3)") "value at position 12: ", c(12)
end program vector_addition
