program laplace2d 
use ISO_FORTRAN_ENV, only : real64, int32
!use openacc
implicit none

! Calculated solution for (E,B) fields
real   (kind=real64), dimension(:,:), allocatable :: T, T_new
! Dimension of the system
integer(kind=int32 )                              :: nx, ny ! number of points

integer(kind=int32 )                              :: i, j, it
real   (kind=real64)                              :: erreur

nx = 20000 !30000
ny = 10000 !30000

allocate(T(nx,ny),T_new(nx,ny))

! initial conditions
do j=2,ny-1
    do i=2,nx-1
        T(i,j)     = 0.0_real64
        T_new(i,j) = 0.0_real64
    enddo
enddo
!
do i=1,nx
    T(i, 1) = 100.0_real64
    T(i,ny) = 0.0_real64
enddo
!
do i=1,ny
    T(1 ,i) = 0.0_real64
    T(nx,i) = 0.0_real64
enddo

!$acc data copy(T) create(T_new)
do it = 1, 10000
  erreur = 0.0_real64
  !$acc parallel loop tile(32,32) reduction(max:erreur)
  do j= 2,ny-1
      do i= 2,nx-1
         T_new(i,j) =  0.25_real64*(T(i+1,j)+T(i-1,j) + &
                                    T(i,j+1)+T(i,j-1))
         erreur = max(erreur, abs(T_new(i,j) - T(i,j)))
      enddo
  enddo

  if (mod(it,100) .eq. 0) print *, "iteration: ",it," erreur: ",erreur

  !$acc parallel loop tile(32,32)
  do j= 2,ny-1
  do i= 2,nx-1
     T(i,j) =  T_new(i,j)
  enddo
  enddo  
enddo
!$acc end data

deallocate(T, T_new)

end program laplace2d
