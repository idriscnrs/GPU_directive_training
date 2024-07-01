program Loop_configuration
    use ISO_FORTRAN_ENV, only : INT32, REAL64, INT64
    implicit none
    integer(kind=INT32 ), parameter              :: system_size = 50000
    real   (kind=REAL64), dimension(system_size) :: array
    real   (kind=REAL64), dimension(system_size) :: table
    real   (kind=REAL64)                         :: sum_val, res, norm, time
    integer(kind=INT32 )                         :: i, j, length, ngangs, numarg
    character(len=:)    , allocatable            :: arg1

    numarg = command_argument_count()
    if (numarg .ne. 1) then
        write(0,*) "Error, you should provide an argument of integer kind to specify the number of gangs that will be used"
        stop
    endif
    call get_command_argument(1,LENGTH=length)
    allocate(character(len=length) :: arg1)
    call get_command_argument(1,VALUE=arg1)
    read(arg1,'(i10)') ngangs

    norm    = 1.0_real64 / (int(system_size, INT64) * int(system_size, INT64))
    res     = 0.0_real64 ! to compare CPU and GPU quickly

    do j = 1, system_size
        sum_val = 0.0_real64
        do i = 1, system_size
            table(i) = (i+j) * norm
        enddo

        do i = 1, system_size
            sum_val  = sum_val + table(i)
        enddo
        array(j) = sum_val
        res = res + sum_val
    enddo

    print *, "result: ",res

!    do i = 1, system_size
!        print *, i,array(i)
!    enddo

end program Loop_configuration
