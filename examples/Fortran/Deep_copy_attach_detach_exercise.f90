module utils_lotka
    use ISO_FORTRAN_ENV, only : REAL64, INT32
    implicit none

    type :: population
        real(kind=REAL64),dimension(:),allocatable :: state
    end type          

    contains
    subroutine derivee(x, dx, pop)
        type   (population ), dimension(2), intent(in)  :: pop 
        real   (kind=REAL64), dimension(2), intent(in)  :: x
        real   (kind=REAL64), dimension(2), intent(out) :: dx
        ! Add openacc directives
        dx(1) =  pop(1)%state(2)*x(1) - pop(1)%state(3)*x(1)*x(2)
        dx(2) = -pop(2)%state(3)*x(2) + pop(2)%state(2)*x(1)*x(2)
    end subroutine derivee

    subroutine rk4(pop, dt)
        type   (population ), dimension(2), intent(inout) :: pop    
        real   (kind=REAL64), intent(in)                  :: dt
        
        real   (kind=REAL64), dimension(2)                :: x_temp, k1, k2, k3, k4
        real   (kind=REAL64)                              :: halfdt        
        integer(kind= INT32)                              :: i

        halfdt = dt/2
        ! Add openacc directives

        do i = 1, 2
           x_temp(i) = pop(i)%state(1)
        enddo

        call Derivee(x_temp, k1, pop)
        ! Add openacc directives
        do i = 1, 2
             x_temp(i) = pop(i)%state(1) + k1(i)*halfdt
        enddo
        
        call Derivee(x_temp, k2, pop)
        ! Add openacc directives
        do i = 1, 2
            x_temp(i) = pop(i)%state(1) + k2(i)*halfdt
        enddo

        call Derivee(x_temp, k3, pop)
        ! Add openacc directives
        do i = 1, 2
            x_temp(i) = pop(i)%state(1) + k3(i)*dt
        enddo

        call Derivee(x_temp, k4, pop)
        ! Add openacc directives
        do i = 1, 2
            pop(i)%state(1) = pop(i)%state(1) + (dt/6.0)*(k1(i) + 2.0*k2(i) + 2.0*k3(i) + k4(i))
        enddo
    end subroutine rk4

end module utils_lotka

program lotka_volterra
    use utils_lotka
    use openacc
    implicit none

    type   (population ), dimension(2)         :: pred_prey
    real   (kind=REAL64)                       :: ti, tf, dt, tmax
    integer                                    :: i
    
    ti   =   0.00 
    dt   =   0.05 
    tmax = 100.00 
    
    allocate(pred_prey(2)%state(3), pred_prey(1)%state(3)) 
    
    pred_prey(2)%state(1) = 15.00  ! predator count
    pred_prey(2)%state(2) = 0.01   ! predator birth rate
    pred_prey(2)%state(3) = 1.0    ! predator death rate

    pred_prey(1)%state(1) = 100.00 ! prey count
    pred_prey(1)%state(2) = 2.0    ! prey birth rate 
    pred_prey(1)%state(3) = 0.02   ! prey death rate

    do i=1,2
    ! Add openacc directives to pass child structure to GPU first
    enddo
    ! Add openacc directives for parent structure and connection to child

    open(unit=42, file="output")
    do while (ti <= tmax)
        tf = ti + dt
        call rk4(pred_prey, dt)
        do i = 1, 2
            ! Add openacc directives
        enddo
        write(42,'(f20.8,a1,f20.8,a1,f20.8)') tf,";", pred_prey(1)%state(1),";", pred_prey(2)%state(1)
        ti = tf
    end do    
    close(42)

    do i=1,2
    ! Add openacc directives 
    enddo
    ! Add openacc directives
end program lotka_volterra    
