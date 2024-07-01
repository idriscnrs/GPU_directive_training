module utils
    use iso_fortran_env, only : REAL64, INT32
    implicit none
    contains
        subroutine init(array, system_size)
            ! Initialize the array to 0.0
            real    (kind=REAL64), allocatable, dimension(:), intent(inout) :: array
            integer (kind=INT32 ), intent(in)                               :: system_size
            integer (kind=INT32 )                                           :: i
            allocate(array(system_size))
            !$acc parallel loop
            do i = 1, system_size
                array(i) = 0.0_real64
            enddo
            !$acc enter data copyin(array(:))
        end subroutine init
        subroutine gaussian(array, system_size, start, finish, exp_val, variance)
            !  Fill an array with the values of gaussian functions        
            real    (kind=REAL64), dimension(:), intent(inout) :: array
            integer (kind=INT32 ), intent(in)                  :: system_size, start, finish
            real    (kind=REAL64), intent(in)                  :: exp_val, variance
            real    (kind=REAL64)                              :: g, pi, norm, step, stepg, steps, position
            integer (kind=INT32 )                              :: i, j, num_gauss

            pi        = acos(-1.0_real64)
            num_gauss = 300
            norm      = 1.0_real64/(variance*sqrt(2*pi))
            step      = dble(finish - start)
            steps     = step / dble(system_size)
            stepg     = step / dble(num_gauss  )
            !$acc parallel present(array(:))
            !$acc loop seq
            do j = 0, num_gauss
                g = start + j*stepg
                !acc loop private(position)
                do i = 1, system_size
                    position =    start + (i-1)*steps
                    array(i) = array(i) + norm*exp(-0.5_real64*(position - g)*(position-g)/(variance*variance))
                enddo
            enddo            
            !$acc end parallel
        end subroutine gaussian

        subroutine add_noise(array, system_size, max_noise)
        ! Take an array and add some noise to the values
        ! max_noise is a fraction of the maximum value of the array 
            real    (kind=REAL64), dimension(:), intent(inout) :: array
            integer (kind=INT32 ), intent(in)                  :: system_size
            real    (kind=REAL64), intent(in)                  :: max_noise
            real    (kind=REAL64)                              :: max_val, rand_val
            integer (kind=INT32 )                              :: i

            max_val = tiny(max_val)
            call srand(32480842)
            ! Find maximum of the function
            !$acc parallel loop reduction(max:max_val) present(array(:))
            do i = 1, system_size
                if (array(i) .gt. max_val) max_val = array(i)
            enddo 

            ! Add noise
            !$acc update self(array(:))
            do i = 1, system_size
               call random_number(rand_val)
               array(i) = array(i) + 2.0_real64*max_noise*max_val*(rand_val-0.5_real64)
            enddo
            !$acc update device(array(:))
        end subroutine add_noise
        real(kind=REAL64) function integral(array, system_size, start, finish)
        ! Compute the integral
            real    (kind=REAL64), dimension(:), intent(inout) :: array
            integer (kind=INT32 ), intent(in)                  :: system_size, start, finish
            real    (kind=REAL64)                              :: step
            integer (kind=INT32 )                              :: i

            step = dble(finish-start)/dble(system_size)
            integral  = 0.0_real64
            !$acc parallel loop reduction(+:integral) present(array(:))            
            do i = 1, system_size
                integral = integral + array(i) + array(i+1)
            enddo
            integral = integral*step*0.5_real64
        end function integral

end module utils

program normalize
    use utils
    real    (kind=REAL64), allocatable, dimension(:)    :: array
    real    (kind=REAL64)                               :: exp_val, variance, gauss_integral, noised_integral
    real    (kind=REAL64)                               :: max_noise
    integer (kind=INT32 )                               :: system_size=20000, start=-5, finish=5

    exp_val  =  0.0_real64
    variance =  1.0_real64 
    call init(array, system_size)
    call gaussian(array, system_size, start, finish, exp_val, variance)
    gauss_integral = integral(array, system_size, start, finish)
    write(0,"(a22,i2,a1,i1,a5,f20.8)") "Gaussian integral on [",start,",",finish,"] is ",gauss_integral

    ! Make some noise !
    max_noise = 0.01_real64
    call add_noise(array, system_size, max_noise)
    noised_integral = integral(array, system_size, start, finish)
    write(0,"(a22,i2,a1,i1,a5,f20.8)") "Gaussian integral on [",start,",",finish,"] is ",noised_integral
    write(0,"(a73,e10.4)") "The integral difference between the gaussian and the noised gaussian is: ", &
                            gauss_integral - noised_integral
    !$acc exit data delete(array)
!    deallocate(array)
end program normalize
