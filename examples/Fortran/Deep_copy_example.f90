program vector_addition
    use iso_fortran_env, only : INT32, REAL64
    use openacc
    implicit none

    type :: vectors
        real(kind=REAL64), dimension(:), allocatable :: s, c
    end type

    type(vectors) :: vec

    integer(kind=INT32 ), parameter              :: system_size = 1e5
    real   (kind=REAL64), dimension(system_size) :: array_sum
    real   (kind=REAL64)                         :: fortran_pi
    integer(kind=INT32 )                         :: i

    fortran_pi = acos(-1.0_real64)
    allocate(vec%s(system_size), vec%c(system_size))

    !$acc enter data create(vec, array_sum(:))
    !$acc enter data create(vec%s(1:system_size), vec%c(1:system_size))


    !$acc parallel
    !$acc loop
    do i = 1, system_size
        vec%s(i) = sin(i*fortran_pi/system_size) * sin(i*fortran_pi/system_size)
        vec%c(i) = cos(i*fortran_pi/system_size) * cos(i*fortran_pi/system_size)
    enddo
    !$acc end parallel

    !$acc parallel
    !$acc loop
    do i = 1, system_size - 1 
        array_sum(i) = vec%s(i) + vec%c(system_size - i)
    enddo
    !$acc end parallel

    !$acc exit data delete(vec%s, vec%c)
    !$acc exit data delete(vec) copyout(array_sum(:))
    
    write(0,"(a10,f10.8)") "sum(42) = ",array_sum(42)
end program vector_addition 
