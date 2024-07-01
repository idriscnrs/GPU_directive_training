program tiles
    use ISO_Fortran_env, only : INT32, REAL64
    implicit none
    integer(kind=INT32), parameter :: ni=4280, nj=4024, nk=1960, ntimes=30
    real(kind=REAL64)              :: a(ni,nk), b(nk,nj), c(ni,nj), d(ni,nj)
    real(kind=REAL64)              :: summation, t1, t2
    integer(kind=INT32)            :: nt, i, j, k
    integer(kind=INT32)            :: ichunk, l, ii, jj, kk   

    call random_number(a)
    call random_number(b)
    
    a = 4.0_real64*a - 2.0_real64
    b = 8.0_real64*b - 4.0_real64
    c = 2.0_real64 
    d = 0.0_real64
    
    print *, "Start calculation"

    call cpu_time(t1)
    do nt = 1, ntimes
      d = c + matmul(a,b)
    end do
    call cpu_time(t2)
    print *, "CPU matmul"
    print *, "elapsed",t2-t1

    print *,sum(d)
    d = 0.0_real64
    print *, " "

       
    call cpu_time(t1)
    do nt = 1, ntimes
      do j=1,nj
         do i =1,ni
           summation = 0.0_real64
           do k=1,nk
              summation = summation + a(i,k) * b(k,j)
           enddo
           d(i,j) = summation + c(i,j)
         enddo
      enddo
    enddo
    call cpu_time(t2)
    print *, "CPU naive loop"
    print *, "elapsed",t2-t1
    print *,sum(d)
    d = 0.0_real64
    print *, " "    


    call cpu_time(t1)    
    l=size(a,dim=2)
    ichunk = 256
    do nt = 1, ntimes
    d = 0.0_real64
    do jj = 1, nj, ichunk
        do kk = 1, l, ichunk
           do j=jj,min(jj+ichunk-1,nj)
             do k=kk,min(kk+ichunk-1,l)
              do i=1,ni
                d(i,j) = d(i,j) + a(i,k) * b(k,j)
              enddo
             enddo
            enddo
         enddo
    enddo
    do j = 1, nj
        do i = 1, ni
            d(i,j) = d(i,j) +c(i,j)
        enddo
    enddo
    enddo
    call cpu_time(t2)
    print *, "CPU manual loop tiling 256"
    print *, "elapsed",t2-t1
    print *,sum(d)
    d = 0.0_real64
    print *, " "

    
    call cpu_time(t1)
    l=size(a,dim=2)
    ichunk = 512
    do nt = 1, ntimes
    d = 0.0_real64
    do jj = 1, nj, ichunk
        do kk = 1, l, ichunk
           do j=jj,min(jj+ichunk-1,nj)
             do k=kk,min(kk+ichunk-1,l)
              do i=1,ni
                d(i,j) = d(i,j) + a(i,k) * b(k,j)
              enddo
             enddo
            enddo
         enddo
    enddo
    do j = 1, nj
        do i = 1, ni
            d(i,j) = d(i,j) +c(i,j)
        enddo
    enddo
    enddo
    call cpu_time(t2)
    print *, "CPU manual loop tiling 512"
    print *, "elapsed",t2-t1
    print *,sum(d)
    d = 0.0_real64

end program tiles

