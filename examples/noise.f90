program NoiseAverage

use, intrinsic :: ISO_FORTRAN_ENV , only : ed_dp   => REAL64, &
                                           ed_sp   => REAL32, &
                                           ed_sint => INT32 , &
                                           ed_dint => INT64 
use openacc
use openacc_curand

implicit none      

real(kind=ed_dp), dimension(:,:slightly_smiling_face:, allocatable :: field

real(kind=ed_dp)                              :: field_average,scaling

type(curandStateXORWOW)                       :: h

integer(kind=ed_dint)                         :: seed,seq,offset
integer(kind=ed_sint)                         :: nx,ny

integer(kind=ed_sint)                         :: i,j

open(unit=21,file="Input")
read(21,) nx
read(21,) ny
close(21)

scaling=nx*ny
scaling=1.0/scaling

allocate(field(nx,ny))
!$acc data create(field)

! Not require 
!$acc parallel loop
do j=1,ny
 !$acc loop
 do i=1,nx
  field(i,j)=0.0_ed_dp
 enddo
enddo

! assigne random field
!$acc parallel loop
do j=1,ny
 seed = 12345 + j
 seq = 0
 offset = 0
 call curand_init(seed, seq, offset, h)
 !$acc loop seq
 do i=1,nx
    field(i,j) = curand_uniform(h)
 enddo
enddo
 
field_average=0.0_ed_dp
!$acc parallel loop reduction(+:field_average)
do j=1,ny
 !a$cc loop reduction(+:field_average)
 do i=1,nx
   field_average=field_average+field(i,j)*scaling
 enddo
enddo

!$acc update host(field)

open(unit=22,file="Output")
write(22,*) field_average
!do j=1,ny
! do i=1,nx
!  write(22,'(2i4,ES24.17)') i,j,field(i,j)
! enddo
!enddo
close(22)

!$acc end data

deallocate(field)

end program NoiseAverage
