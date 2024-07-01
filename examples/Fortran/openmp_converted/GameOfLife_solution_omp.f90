program gol
    implicit none
    integer :: rows, cols, generations
    integer, allocatable :: world(:,:), old_world(:,:)
    integer :: g
    integer :: long
    character(len=:), allocatable :: arg
    integer :: i
    integer, dimension(3) :: n
    ! Read cmd line args
    DO i=1, COMMAND_ARGUMENT_COUNT()
       CALL GET_COMMAND_ARGUMENT(NUMBER=i, LENGTH=long)
       ALLOCATE(CHARACTER(len=long) :: arg)
       CALL GET_COMMAND_ARGUMENT(NUMBER=i, VALUE=arg)
       READ(arg,'(i10)') n(i)
       DEALLOCATE(arg)
    END DO
    rows=n(1)
    cols=n(2)
    generations=n(3)
    allocate(world(0:rows+1,0:cols+1), old_world(0:rows+1,0:cols+1))
    old_world(:,:) = 0
    call fill_world
    !$ACC enter data copyin(world, old_world)
!$omp target enter data map(to:world,old_world)
    do g=1,generations
        call save_world(rows,cols, world(:,:), old_world(:,:))
        call next(rows,cols, world(:,:), old_world(:,:))
        print *, "Cells alive at generation ", g, ": ", alive(rows, cols, world)
        call output_world("generation", g, world(:,:), rows, cols)
    enddo
    !$ACC exit data delete(world, old_world)
!$omp target exit data map(delete:world,old_world)
    deallocate(world, old_world)
    contains
    integer function alive(rows, cols, world) result(cells)
        implicit none
        integer, intent(in) :: rows, cols
        integer, intent(in) :: world(0:rows+1,0:cols+1)
        integer :: r,c
        cells = 0
        !$ACC parallel loop reduction(+:cells)
!$omp target teams loop reduction(+:cells)
        do r=1, rows
            do c=1, cols
               cells = cells + world(r,c) 
            enddo
        enddo
        end function alive
    subroutine save_world(rows,cols,world, old_world)
        implicit none
        integer, intent(in) :: rows,cols
        integer, intent(in) :: world(0:rows+1,0:cols+1)
        integer, intent(out) :: old_world(0:rows+1,0:cols+1)
        integer :: r,c
        !$ACC parallel loop
!$omp target teams loop
        do r=1, rows
            do c=1, cols
                old_world(r,c) = world(r,c)
            enddo
        enddo
    end subroutine save_world

    subroutine next(rows,cols,world, old_world)
        implicit none
        integer, intent(in) :: rows,cols
        integer, intent(in) :: old_world(0:rows+1,0:cols+1)
        integer, intent(out) :: world(0:rows+1,0:cols+1)
        integer :: r,c
        integer :: neigh
        !$ACC parallel loop
!$omp target teams loop
        do r=1, rows
            do c=1, cols
        neigh = old_world(r-1,c-1)+old_world(r,c-1)+old_world(r+1,c-1)+&
                old_world(r-1,c)+old_world(r+1,c)+&
                old_world(r-1,c+1)+old_world(r,c+1)+old_world(r+1,c+1)
                if (old_world(r,c) == 1 .and. (neigh<2.or.neigh>3) ) then
                    world(r,c) = 0
                else if (neigh == 3) then
                    world(r,c) = 1
                endif
            enddo
        enddo
    end subroutine next

    subroutine fill_world
        implicit none
        integer :: r,c, temp
        real*8 :: test
        do r=1,rows
            do c=1,cols
                call random_number(test)
                temp = mod(floor(test*100),4)
                if (temp.eq.0) then 
                    temp = 1
                else
                    temp = 0
                endif
                world(r,c) = temp
            enddo
        enddo
    end subroutine fill_world
    
    subroutine output_world(name, g, world, rows, cols)
        implicit none
        character(len=*), intent(in) :: name
        character(len=1024) :: filename
        integer, intent(in) :: g
        integer, intent(in) :: rows,cols
        integer, intent(in) :: world(0:rows+1,0:cols+1)
        integer :: r,c
        integer :: my_unit
        integer(kind=1) :: output(0:rows+1,0:cols+1)
        write(filename,"(A,I5.5,A)") trim(name), g,".gray"
        print *, trim(filename)
        !$ACC update self(world)
!$omp target update from(world)
        do r=0,rows+1
            do c=0,cols+1
                output(r,c) = mod(world(r,c),2)*127
            enddo
        enddo
        open(newunit=my_unit, file=filename, access="stream", form="unformatted")
        write(my_unit, pos=1) output
        close(my_unit)
    end subroutine output_world
end program gol

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp GameOfLife_solution.f90
