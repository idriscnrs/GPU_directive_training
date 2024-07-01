program reduction_exponential
    use iso_fortran_env, only : INT32, REAL64
    implicit none
    ! current position and values
    real   (kind=REAL64) :: x, y, x_p
    real   (kind=REAL64) :: double_min, double_max, begin, fortran_pi, summation
    real   (kind=REAL64) :: step_l
    ! number of division of the function
    integer(kind=INT32 ) :: nsteps 
    integer(kind=INT32 ) :: i

    nsteps     = 1e9
    begin      = 0.0_real64                  ! x min 
    fortran_pi = acos(-1.0_real64)           ! x max
    summation  = 0.0_real64                  ! sum of elements
    step_l     = (fortran_pi - begin)/nsteps ! length of the step

    double_min = huge(double_min)
    double_max = tiny(double_max)
    !$acc parallel loop reduction(+:summation) reduction(min:double_min) reduction(max:double_max)
!$omp target teams loop reduction(+:summation)&
!$omp reduction(min:double_min) reduction(max:double_max)
    do i = 1, nsteps
        x   =  i * step_l
        x_p = (i+1) * step_l
        y   = (exp(x)+exp(x_p))*0.5_real64
        summation = summation + y
        if (y .lt. double_min) double_min = y
        if (y .gt. double_max) double_max = y
    enddo

    ! print the stats
    write(0,"(a38,f20.8)") "The MINimum value of the function is: ",double_min
    write(0,"(a38,f20.8)") "The MAXimum value of the function is: ",double_max
    write(0,"(a33,f3.1,a1,f8.6,a6,f20.8)") "The integral of the function on [",begin, &
                                           ",",fortran_pi,"] is: ",summation*step_l
    write(0,"(a18,es20.8)") "   difference is: ",exp(fortran_pi)-exp(begin)-summation*step_l
end program reduction_exponential

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp reduction_exponential_solution.f90
