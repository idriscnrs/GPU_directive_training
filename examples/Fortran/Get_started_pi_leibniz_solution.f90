program pi_leibniz
    use iso_fortran_env, only : INT32, REAL64
    use openacc
    implicit none

    real   (kind=REAL64) :: pi, diff, fortran_pi, a, b, c, d
    integer(kind=INT32 ) :: i , steps 
 
    pi   = 0.0_real64
    diff = 0.0_real64

    fortran_pi = acos(-1.0_real64)

    steps = huge(i)/2

    !$acc parallel loop reduction(+:pi)
    do i = 0, steps
         pi = pi + ( (-1)**i )/(dble(2*i+1))
    enddo

    diff = 4.0_real64 * pi - fortran_pi
    
    write(0,"(a32,e10.4,a6,i10,a6)") "Difference with pi from acos is ",diff," with ",steps," steps"
    if (abs(diff) .lt. 1.0e-5 ) then
        write(0,*) "Results seems correct"
    else
        write(0,*) "Results seems wrong"
    endif
end program pi_leibniz

