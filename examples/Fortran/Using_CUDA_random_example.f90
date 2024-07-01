program using_cuda
    use openacc
    use openacc_curand
    use, intrinsic :: ISO_FORTRAN_ENV , only : REAL32, INT32 
    implicit none      

    integer(kind=INT32), dimension(:), allocatable :: shots
    integer(kind=INT32)                            :: histo(10)
    integer(kind=INT32)                            :: nshots
    type(curandStateXORWOW)                        :: h
    integer(kind=INT32)                            :: seed,seq,offset    
    integer(kind=INT32)                            :: i

    do i = 1, 10
        histo(i) = 0
    enddo

    nshots = 1e9
    !  Allocate memory for the random numbers
    allocate(shots(nshots))

    ! NVIDIA curand will create our initial random data
    !$acc parallel create(shots(:)) copyout(histo(:))
    seed = 1234!5 + j
    seq = 0
    offset = 0
    !$acc loop vector
    do i = 1, 32
        call curand_init(seed, seq, offset, h)
    enddo

    !$acc loop
    do i = 1, nshots
        shots(i) = abs(curand(h))
    enddo

    !  Count the number of time each number was drawn
    !$acc loop
    do i = 1, nshots
        shots(i) = mod(shots(i),10) + 1
        !$acc atomic update
        histo(shots(i)) = histo(shots(i)) + 1
    enddo
    !$acc end parallel
    !  Print results
    do i = 1, 10 
        write(0,"(i2,a2,i10,a2,f5.3,a1)") i,": ",histo(i)," (",dble(histo(i))/dble(1e9),")" 
    enddo
    deallocate(shots)
end program using_cuda
