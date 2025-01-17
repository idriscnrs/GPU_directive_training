program vector_addition
    use iso_fortran_env, only : INT32, REAL64
    use openacc
    implicit none

    integer(kind=INT32 ), parameter              :: system_size = 1e5
    real   (kind=REAL64), dimension(system_size) :: s, c, array_sum
    real   (kind=REAL64)                         :: fortran_pi
    integer(kind=INT32 )                         :: i

    fortran_pi = acos(-1.0_real64)

    !$acc parallel

    !$acc loop
    do i = 1, system_size
        s(i) = sin(i*fortran_pi/system_size) * sin(i*fortran_pi/system_size)
        c(i) = cos(i*fortran_pi/system_size) * cos(i*fortran_pi/system_size)
    enddo
    !$acc end parallel

    !$acc parallel

    !$acc loop
    do i = 1, system_size - 1
        array_sum(i) = s(i) + c(system_size - i)
    enddo
    !$acc end parallel
    
    write(0,"(a10,f10.8)") "sum(42) = ",array_sum(42)
end program vector_addition 
