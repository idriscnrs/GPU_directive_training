module utils
    use ISO_FORTRAN_ENV, only : REAL64, INT32
    implicit none

    type :: Force
        real(kind=REAL64), dimension(:), allocatable :: Fx, Fy, Fz
    end type
    type :: velocity
        real(kind=REAL64), dimension(:), allocatable :: vx, vy, vz
    end type
    type :: Pos
        real(kind=REAL64), dimension(:), allocatable :: x, y, z
    end type

    type :: atom
        type(Force)        :: Fext
        type(Velocity)     :: V
        type(Pos)          :: R
        real (kind=REAL64) :: m0
    end type

    type(atom)           :: particle
    ! position parameters
    integer(kind= INT32) :: Natoms
    real   (kind=REAL64) :: Lx, Ly, Lz

    ! potential and force parameters
    real   (kind=REAL64) :: epsilon0, sigma0    
    real   (kind=REAL64) :: LJ_tolerance = 1e-9
    real   (kind=REAL64) :: rcut = 15.0_real64

    ! temporal parameters
    integer(kind= INT32) :: it_max
    real   (kind=REAL64) :: dt, end_time, print_config, print_thermo, start_time=0.0_real64
    integer(kind= INT32) :: write_restart = 0, read_restart = 0

    ! Thermodynamic parameters
    real   (kind=REAL64) :: Ek, T, T0 ! kinetic energy & temperature from simulation, T0 = targeted temperature
    real   (kind=REAL64), parameter :: kb = 0.831451115

    ! Berendsen thermostat
    real   (kind=REAL64) :: lambda_scaling_factor, tau_temp

    contains
    subroutine forces_from_LJ_potential()
        real   (kind=REAL64) :: rij, xij, yij, zij
        real   (kind=REAL64) :: Fxij, Fyij, Fzij, sigma1, sr2, sr6, sr12
        integer(kind= INT32) :: i, j, divergence = 0
        sigma1   = sigma0*sigma0

        ! Add open acc directives

        do i = 1, Natoms
            Fxij = 0.0_real64
            Fyij = 0.0_real64
            Fzij = 0.0_real64
            ! Add open acc directives
            do j = 1, Natoms ! avoid triangular matrix on gpu
            
            if (i .ne. j) then
                xij = particle%R%x(j)-particle%R%x(i)
                xij = xij - anint(xij/Lx) * Lx
                yij = particle%R%y(j)-particle%R%y(i)
                yij = yij - anint(yij/Lz) * Lz
                zij = particle%R%z(j)-particle%R%z(i)
                zij = zij - anint(zij/Lz) * Lz

                rij = xij*xij + yij*yij + zij*zij
                if ((rij .gt. LJ_tolerance) .and. (rij .le. rcut)) then
                    sr2  = sigma1 / rij
                    sr6  = sr2 *  sr2 * sr2
                    sr12 = sr6 * sr6
                    sr6  = sr6 / rij
                    sr12 = sr12 / rij
                    Fxij = (sr12 - 24_real64 * sr6) * xij + Fxij
                    Fyij = (sr12 - 24_real64 * sr6) * yij + Fyij
                    Fzij = (sr12 - 24_real64 * sr6) * zij + Fzij
                else 
                        if (rij .lt. LJ_tolerance) then
                                divergence = divergence + 1  ! avoid early break on GPU
                        endif
                endif

            endif
            enddo
            particle%Fext%Fx(i) = 48.0_real64*epsilon0*Fxij
            particle%Fext%Fy(i) = 48.0_real64*epsilon0*Fyij
            particle%Fext%Fz(i) = 48.0_real64*epsilon0*Fzij
        enddo

        if (divergence .ne. 0) then
             write(0,*) 'Particles are too close'
             stop
        endif

    end subroutine forces_from_LJ_potential

    subroutine velocity_verlet()
        integer(kind= INT32) :: i
        
        ! Add open acc directives

        do i = 1, Natoms
            particle%V%vx(i) = particle%V%vx(i) + 0.5_real64 * dt * particle%Fext%Fx(i)
            particle%V%vy(i) = particle%V%vy(i) + 0.5_real64 * dt * particle%Fext%Fy(i)
            particle%V%vz(i) = particle%V%vz(i) + 0.5_real64 * dt * particle%Fext%Fz(i)
        
            particle%R%x(i) = particle%R%x(i) + dt*particle%V%vx(i)
            particle%R%y(i) = particle%R%y(i) + dt*particle%V%vy(i)
            particle%R%z(i) = particle%R%z(i) + dt*particle%V%vz(i)
            particle%R%x(i) = particle%R%x(i) - anint(particle%R%x(i)/Lx) * Lx
            particle%R%y(i) = particle%R%y(i) - anint(particle%R%y(i)/Ly) * Ly
            particle%R%z(i) = particle%R%z(i) - anint(particle%R%z(i)/Lz) * Lz
        enddo

        call forces_from_LJ_potential()

        ! Add open acc directives
        do i = 1, Natoms
            particle%V%vx(i) = particle%V%vx(i) + 0.5_real64 * dt * particle%Fext%Fx(i)
            particle%V%vy(i) = particle%V%vy(i) + 0.5_real64 * dt * particle%Fext%Fy(i)
            particle%V%vz(i) = particle%V%vz(i) + 0.5_real64 * dt * particle%Fext%Fz(i)
        enddo
    end subroutine velocity_verlet

    subroutine berendsen_thermostat()
        integer(kind= INT32) :: i
        ! Add open acc directives
        Ek = 0.0_real64

        ! Add open acc directives
        do i = 1, Natoms 
            Ek = Ek + particle%m0 * (particle%V%vx(i)*particle%V%vx(i) + &
                                     particle%V%vy(i)*particle%V%vy(i) + &
                                     particle%V%vz(i)*particle%V%vz(i))
        enddo
        Ek = 0.5_real64 * Ek

        T = 2.0_real64 * kb * Ek / (3.0_real64 * Natoms -3)          
        lambda_scaling_factor = sqrt(1 + dt * (-1 + T0/T) / tau_temp)
        ! Add open acc directives
        do i = 1, Natoms
            particle%V%vx(i) = lambda_scaling_factor * particle%V%vx(i)
            particle%V%vy(i) = lambda_scaling_factor * particle%V%vy(i)
            particle%V%vz(i) = lambda_scaling_factor * particle%V%vz(i)
        enddo
        Ek = Ek * lambda_scaling_factor**2
    end subroutine berendsen_thermostat

    subroutine write_config(it)
        real   (kind=REAL64), intent(inout) :: it
        integer(kind= INT32)                :: i
        ! Add open acc directives
        open(unit=20, file='OUTPUT')
        write(20,*) it, Natoms, Lx, Ly, Lz
        ! Add open acc directives
        if (read_restart .eq. 1) then
                ! Add open acc directives
        endif
        if (read_restart .eq. 2) then
                ! Add open acc directives
        endif

        do i = 1, Natoms
            write(20,*) particle%R%x(i), particle%R%y(i), particle%R%z(i)
            if (read_restart .eq. 1) write(20,*) particle%V%vx(i), particle%V%vy(i), particle%V%vz(i)
            if (read_restart .eq. 2) write(20,*) particle%Fext%Fx(i), particle%Fext%Fy(i), particle%Fext%Fz(i)
        enddo
        close(20)
    end subroutine write_config

    subroutine read_config(it)
        real   (kind=REAL64), intent(out) :: it
        integer(kind= INT32)              :: i
        open(unit=20, file='CONFIG')
        read(20,*) it, Natoms, Lx, Ly, Lz
        call allocate_variables()
        ! Add open acc directives
        do i = 1, Natoms
            read(20,*) particle%R%x(i), particle%R%y(i), particle%R%z(i)
            if (read_restart .eq. 1) then
                   read(20,*) particle%V%vx(i), particle%V%vy(i), particle%V%vz(i)
            endif
            if (read_restart .eq. 2) then
                   read(20,*) particle%Fext%Fx(i), particle%Fext%Fy(i), particle%Fext%Fz(i)
            endif
        enddo

        ! Add open acc directives
        if (read_restart .eq. 1) then
                ! Add open acc directives
        endif
        if (read_restart .eq. 2) then
                ! Add open acc directives
        endif

        close(20)
        particle%m0 = 1.0
        ! Add open acc directives
    end subroutine read_config

    subroutine allocate_variables
        allocate(particle%R%x(Natoms), particle%R%y(Natoms), particle%R%z(Natoms))
        allocate(particle%V%vx(Natoms), particle%V%vy(Natoms), particle%V%vz(Natoms))
        allocate(particle%Fext%Fx(Natoms), particle%Fext%Fy(Natoms), particle%Fext%Fz(Natoms))
        ! Add open acc directives
    end subroutine allocate_variables

    subroutine read_params
        namelist/TIME_CONFIG/ dt, end_time, print_config, print_thermo, write_restart, read_restart
        namelist/LJ_CONFIG/   sigma0, epsilon0
        namelist/BERENDSEN_CONFIG/ T0, tau_temp
  
        open( 11,file='INPUT_NML',status='old')
        read( 11,NML=TIME_CONFIG)
        read( 11,NML=LJ_CONFIG)
        read( 11,NML=BERENDSEN_CONFIG)
        close(11)
    end subroutine read_params
end module utils

program dm
    use utils

    integer(kind= INT32) :: it, it_print, it_thermo
    real   (kind=REAL64) :: tp

    call read_params()
    call read_config(start_time)
    it_max   = int( (end_time-start_time) /dt)
    it_print = int( print_config/dt)
    it_thermo= int( print_thermo/dt)
    
    open(unit=42, file="THERMO")
    do it = 1, it_max
        call forces_from_LJ_potential()
        call velocity_verlet()
        call berendsen_thermostat()

        if (mod(it,it_print) .eq. 0) then
            tp = it*dt    
            call write_config(tp)
        endif
        if (mod(it,it_thermo) .eq. 0) write(42,*) it*dt, Ek, T
    enddo
    close(42)

    ! Add open acc directives
    deallocate(particle%R%x , particle%R%y , particle%R%z)
    deallocate(particle%V%vx, particle%V%vy, particle%V%vz)
    deallocate(particle%Fext%Fx, particle%Fext%Fy, particle%Fext%Fz)    
end program dm
