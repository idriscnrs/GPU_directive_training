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
        !$acc data present(x(:), dx(:), pop(:), pop(1)%state(:), pop(2)%state(:))
        !$acc serial
        dx(1) =  pop(1)%state(2)*x(1) - pop(1)%state(3)*x(1)*x(2)
        dx(2) = -pop(2)%state(3)*x(2) + pop(2)%state(2)*x(1)*x(2)
        !$acc end serial
        !$acc end data
    end subroutine derivee

    subroutine rk4(pop, dt)
        type   (population ), dimension(2), intent(inout) :: pop    
        real   (kind=REAL64), intent(in)                  :: dt
        
        real   (kind=REAL64), dimension(2)                :: x_temp, k1, k2, k3, k4
        real   (kind=REAL64)                              :: halfdt        
        integer(kind= INT32)                              :: i

        halfdt = dt/2
        !$acc data create(k1(:), k2(:), k3(:), k4(:), x_temp(:)) present(pop(:), pop(1)%state(:), pop(2)%state(:))

        !$acc parallel loop
        do i = 1, 2
           x_temp(i) = pop(i)%state(1)
        enddo

        call Derivee(x_temp, k1, pop)
        !$acc parallel loop
        do i = 1, 2
             x_temp(i) = pop(i)%state(1) + k1(i)*halfdt
        enddo
        
        call Derivee(x_temp, k2, pop)
        !$acc parallel loop
        do i = 1, 2
            x_temp(i) = pop(i)%state(1) + k2(i)*halfdt
        enddo

        call Derivee(x_temp, k3, pop)
        !$acc parallel loop
        do i = 1, 2
            x_temp(i) = pop(i)%state(1) + k3(i)*dt
        enddo

        call Derivee(x_temp, k4, pop)
        !$acc parallel loop
        do i = 1, 2
            pop(i)%state(1) = pop(i)%state(1) + (dt/6.0)*(k1(i) + 2.0*k2(i) + 2.0*k3(i) + k4(i))
        enddo
       !$acc end data
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
       !$acc enter data copyin(pred_prey(i)%state(:))
    enddo
    !$acc enter data copyin(pred_prey(1:2)) attach(pred_prey(1)%state(:), pred_prey(2)%state(:))

    open(unit=42, file="output_solution")
    do while (ti <= tmax)
        tf = ti + dt
        call rk4(pred_prey, dt)
        do i = 1, 2
            !$acc update self(pred_prey(i)%state(1))
        enddo
        write(42,'(f20.8,a1,f20.8,a1,f20.8)') tf,";", pred_prey(1)%state(1),";", pred_prey(2)%state(1)
        ti = tf
    end do    
    close(42)

    do i=1,2
        !$acc exit data detach(pred_prey(i)%state)
        !$acc exit data delete(pred_prey(i)%state)
    enddo
    !$acc exit data delete(pred_prey(:))

end program lotka_volterra    
