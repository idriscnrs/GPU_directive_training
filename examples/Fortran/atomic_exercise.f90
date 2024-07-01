program histogram
    use iso_fortran_env, only : REAL64, INT32
    implicit none

    integer(kind=INT32 ), dimension(:) , allocatable :: shots
    integer(kind=INT32 ), dimension(10)              :: histo
    integer(kind=INT32 ), parameter                  :: nshots = 1e9
    real   (kind=REAL64)                             :: random_real
    integer(kind=INT32 )                             :: i

    ! Histogram allocation and initialization
    do i = 1, 10
     histo(i) = 0
    enddo

    ! Allocate memory for the random numbers
    allocate(shots(nshots))

    ! Fill the array on the CPU (rand is not available on GPU with Nvidia Compilers)
    do i = 1, nshots
        call random_number(random_real)
        shots(i) = floor(random_real * 10.0_real64) + 1
    enddo

    ! Count the number of time each number was drawn
    do i = 1, nshots
        histo(shots(i)) = histo(shots(i)) + 1
    enddo

    !  Print results
    do i = 1, 10
        write(0,"(i2,a2,i10,a2,f10.8,a1)") i,": ", histo(i), " (", real(histo(i))/1.e9, ")"
    enddo

    deallocate(shots)

end program histogram
