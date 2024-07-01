module calcul
    use iso_fortran_env, only : INT32, REAL64
    contains
        subroutine rand_init(array,n)
            real   (kind=REAL64), dimension(1,n), intent(inout) :: array
            integer(kind=INT32 ), intent(in)                    :: n
            real   (kind=REAL64)                                :: rand_val
            integer(kind=INT32)                                 :: i

            call srand(12345900)
            do i = 1, n
               call random_number(rand_val)
               array(1,i) = 2.0_real64*(rand_val-0.5_real64)
            enddo
        end subroutine rand_init

        subroutine iterate(array, array_size, cell_size)
            real   (kind=REAL64), dimension(1:array_size,1), intent(inout) :: array
            integer(kind=INT32 ), intent(in)                               :: array_size, cell_size
            real   (kind=REAL64)                                           :: local_mean
            integer(kind=INT32 )                                           :: i
 
            
            do i = cell_size/2, array_size-cell_size/2
                local_mean = mean_value(array(i+1-cell_size/2:i+cell_size/2,1), cell_size)
                if (local_mean .lt. 0.0_real64) then
                    array(i,1) = array(i,1) + 0.1
                else
                    array(i,1) = array(i,1) - 0.1
                endif
            enddo
        end subroutine iterate

        function mean_value(t, n)
            real   (kind=REAL64), dimension(n,1), intent(inout) :: t
            integer(kind=INT32 ), intent(in)                    :: n
            real   (kind=REAL64)                                :: mean_value
            integer(kind=INT32 )                                :: i
            mean_value = 0.0_real64
            
            do i = 1, n
                mean_value = mean_value + t(i,1)
            enddo
            mean_value = mean_value / dble(n)
        end function mean_value
end module calcul
program modular_programming
    use calcul
    implicit none    
    
    real   (kind=REAL64), dimension(:,:), allocatable :: table
    real   (kind=REAL64), dimension(:)  , allocatable :: mean_values
    integer(kind=INT32 )                              :: nx, ny, cell_size, i

    nx =   10000
    ny =    3000
    allocate(table(nx,ny), mean_values(ny))
    table(:,:) = 0.0_real64
    call rand_init(table(1,:),ny)
    cell_size = 32
    do i = 2, ny   
        call iterate(table(:,i), nx, cell_size)
    enddo

    do i = 1, ny
        mean_values(i) = mean_value(table(:,i), nx)
    enddo

    do i = 1, 10
        write(0,"(a18,i5,a1,f20.8)") "Mean value of row ",i,"=",mean_values(i)
    enddo

    do i = ny-10, ny
        write(0,"(a18,i5,a1,f20.8)") "Mean value of row ",i,"=",mean_values(i)
    enddo    
    
    deallocate(table, mean_values)
end program modular_programming
