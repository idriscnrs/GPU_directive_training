program using_cuda
    use ISO_C_BINDING       
    use openacc
    use, intrinsic :: ISO_FORTRAN_ENV , only : REAL32, INT32 
    implicit none      

    interface
        subroutine fill_rand(positions, length, stream) BIND(C,NAME='fill_rand')
            use ISO_C_BINDING
            use openacc
            implicit none
            type (C_PTR)   , value          :: positions
            integer (C_INT), value          :: length
            integer(acc_handle_kind), value :: stream
        end subroutine fill_rand
     end interface


    integer(C_INT), dimension(:), allocatable :: shots
    integer(C_INT)                            :: histo(10)
    integer(C_INT)                            :: nshots
    integer(acc_handle_kind)                  :: stream

    integer(kind=INT32)                       :: i,j

    do i = 1, 10
        histo(i) = 0
    enddo

    nshots = 1e9
    !  Allocate memory for the random numbers
    allocate(shots(nshots))

    ! OpenACC may not use the default CUDA stream so we must query it
    stream = acc_get_cuda_stream(acc_async_sync)

    !$acc data create(shots(:)) copyout(histo(:))
    ! NVIDIA cuRandom will create our initial random data
    !$acc host_data use_device(shots)
    call fill_rand(C_LOC(shots), nshots, stream)
    !$acc end host_data

    !  Count the number of time each number was drawn
    !$acc parallel loop present(shots(:), histo(:))
    do i = 1, nshots
        shots(i) = mod(shots(i),10) + 1
        !$acc atomic update
        histo(shots(i)) = histo(shots(i)) + 1
    enddo
    !$acc end data
    !  Print results
    do i = 1, 10 
        write(0,"(i2,a2,i10,a2,f5.3,a1)") i,": ",histo(i)," (",dble(histo(i))/dble(1e9),")" 
    enddo
    deallocate(shots)
end program using_cuda
