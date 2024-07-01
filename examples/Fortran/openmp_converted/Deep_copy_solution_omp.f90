module utils_rdf
    use ISO_FORTRAN_ENV, only : REAL64, INT32
    implicit none

    type :: Location
        real(kind=REAL64), dimension(:), allocatable :: x, y, z
    end type
! !$omp declare mapper (Location::x) map ( &
! !$omp  x%x &
! !$omp , x%y &
! !$omp , x%z &
! !$omp )

    type(Location) :: particle
    integer(kind= INT32), dimension(:), allocatable :: hist
    real   (kind=REAL64), dimension(:), allocatable :: gr
    real   (kind=REAL64) :: Rij, deltaR, rcut, nideal, rho, Lx, Ly, Lz
    integer(kind= INT32) :: d, read_restart, Natoms

    contains
    subroutine read_config()
        integer(kind= INT32)              :: i, ierr
        real   (kind=REAL64)              :: yy
        logical                           :: res = .false.
        inquire(file="OUTPUT", exist=res) 
        if (res) then
            open(unit=20, file='OUTPUT', iostat=ierr)    
            write(0,*) "Reading from OUTPUT file"
        else     
            open(unit=20, file='CONFIG', iostat=ierr)
            write(0,*) "Reading from CONFIG file"
        endif
        read(20,*) yy, Natoms, Lx, Ly, Lz

        allocate(particle%x(Natoms), particle%y(Natoms), particle%z(Natoms))
      
        do i = 1, Natoms
            read(20,*) particle%x(i), particle%y(i), particle%z(i)
            if (read_restart .eq. 1) read(20,*) yy, yy, yy
            if (read_restart .eq. 2) read(20,*) yy, yy, yy
        enddo
        close(20)
        !$acc enter data copyin(particle)
!$omp target enter data map(to:particle)
        !$acc enter data copyin(particle%x(1:Natoms), particle%y(1:Natoms), particle%z(1:Natoms))
!$omp target enter data map(to:particle%x(1:natoms),&
!$omp particle%y(1:natoms),particle%z(1:natoms))
    end subroutine read_config

    subroutine usage
    ! Only prints how to run the program
    print *, "You should provide three arguments to run rdf : "
    print *, "./rdf deltaR rcut read_restart"
    print *, "- deltaR is the length of each bin, represented by a real*8 "
    print *, "- rcut is the total length on which you determine the rdf ( rcut < box_length / 2 )"
    print *, "- read_restart defined if the file contains :"
    print *, "   - positions and velocities         (read_restart = 1)"
    print *, "   - positions, velocities and forces (read_restart2)"
    print *, "   - position only (input anything that is not 1 or 2)"                         
    print *, "example : ./rdf 0.5 15.5 0"
    stop
    end subroutine usage

end module utils_rdf

program rdf
    use utils_rdf
    integer(kind= INT32)          :: i, j, nargs, long, max_bin
    character(len=:), allocatable :: arg

    nargs = COMMAND_ARGUMENT_COUNT()
    if (nargs .eq. 3) then    
        call GET_COMMAND_ARGUMENT(NUMBER=1, LENGTH=long)
        allocate(CHARACTER(len=long) :: arg)
        call GET_COMMAND_ARGUMENT(NUMBER=1, VALUE=arg)
        read(arg,'(f20.8)') deltaR
        deallocate(arg)
        call GET_COMMAND_ARGUMENT(NUMBER=2, LENGTH=long)
        allocate(CHARACTER(len=long) :: arg)
        call GET_COMMAND_ARGUMENT(NUMBER=2, VALUE=arg)
        read(arg,'(f20.8)') rcut
        deallocate(arg)
        call GET_COMMAND_ARGUMENT(NUMBER=3, LENGTH=long)
        allocate(CHARACTER(len=long) :: arg)
        call GET_COMMAND_ARGUMENT(NUMBER=3, VALUE=arg)
        read(arg,'(i10)') read_restart
        deallocate(arg)
    else
        call usage()
    endif

    call read_config()
    max_bin = int( rcut/deltaR ) + 1
    allocate(hist(max_bin), gr(max_bin))

    !$acc data copy(hist(:)) copyout(gr(:)) present(particle, particle%x(1:Natoms), particle%y(1:Natoms), particle%z(1:Natoms))
!$omp target data map(tofrom:hist(:)) map(from:gr(:)) 
    !$acc parallel loop
!$omp target teams loop
    do i = 1, max_bin
        hist(i) = 0
    enddo
     
    !$acc parallel loop reduction(+:hist)
!$omp target teams loop reduction(+:hist)
    do j = 1, Natoms
        !$acc loop
!$omp loop
        do i = 1, Natoms
            if (j .ne. i) then
                xij = particle%x(j)-particle%x(i)
                xij = xij - anint(xij/Lx) * Lx
                yij = particle%y(j)-particle%y(i)
                yij = yij - anint(yij/Lz) * Lz
                zij = particle%z(j)-particle%z(i)
                zij = zij - anint(zij/Lz) * Lz

                Rij = xij*xij + yij*yij + zij*zij
                d = int( sqrt(Rij)/deltaR ) + 1

                if (d .le. max_bin) then
                    !$acc atomic update
!$omp atomic update
                    hist(d) = hist(d) + 1
                endif
            endif
        enddo
    enddo

    rho = dble(Natoms) / (Lx * Ly * Lz)
    rho =  4.0_real64 / 3.0_real64 * acos(-1.0_real64) * rho
    
    !$acc parallel loop 
!$omp target teams loop
    do i = 1, max_bin
        nideal  = rho * ( (i*deltaR)**3 - ((i-1)*deltaR)**3)
        gr(i)   = dble(hist(i)) / (nideal * dble(Natoms))
    enddo

    !$acc end data
!$omp end target data
    !$acc exit data delete(particle%x, particle%y, particle%z)
!$omp target exit data map(delete:particle%x,particle%y,particle%z)
    !$acc exit data delete(particle)
!$omp target exit data map(delete:particle)

    open(unit=30, file='RDF')
        do i=1,max_bin
            write(30,'(2f20.8)') (i-1)*deltaR, gr(i)
        enddo
    close(30)

    deallocate(particle%x, particle%y, particle%z)

end program rdf

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Deep_copy_solution.f90
