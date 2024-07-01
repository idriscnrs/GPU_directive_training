program tiles_optimization
    use ISO_FORTRAN_ENV, only : INT32, REAL64
    implicit none

    integer(kind=INT32 ), parameter        :: nx = 10000, ny = 10000 ! put nx, ny = 20000 on V100-32
    real   (kind=REAL64), dimension(nx,ny) :: A
    real   (kind=REAL64), dimension(ny,nx) :: B
    integer(kind=INT32 )                   :: i, j

    !$acc data copyout(A(:,:), B(:,:))
    
    !$acc parallel loop
    do j = 1, ny
        do i = 1, nx
            A(i,j) = i + (j-1)*ny
        enddo
    enddo    

    !$acc parallel loop tile(32,32)
    do j = 1, nx
        do i = 1, ny 
            B(i,j) = A(j,i)
        enddo
    enddo
    !$acc end data

    write(0,'(a2)') 'A:'
    do i = 1, 50
        write(0,"(5f20.8)") A(1:5,i)
    enddo
   
    write(0,'(a2)') 'B:'
    do i = 1, 50
        write(0,"(5f20.8)") B(1:5,i)
    enddo

end program tiles_optimization      
