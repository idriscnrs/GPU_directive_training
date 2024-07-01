program normalize
    use ISO_FORTRAN_ENV, only : INT32, REAL64
    implicit none

    integer(kind=INT32)                             :: mat_rank
    real(kind=REAL64), allocatable, dimension(:,:)  :: A
    integer(kind=INT32)                             :: virt_loop, i
    real(kind=REAL64)                               :: c

    mat_rank = 3000

    allocate(A(mat_rank, mat_rank))

    call random_init(A, mat_rank)

    ! Do not parallelize this loop
    do virt_loop=1, 1
        do i = 1, mat_rank
            call normalize(A(:,i), mat_rank)
        end do
    end do

    print *, "Rank of the matrix", mat_rank, "check is ", check(A, mat_rank)

    deallocate(A)

    contains

        subroutine random_init(A, mat_rank)
            implicit none
            integer(kind=INT32)                             :: mat_rank
            real(kind=REAL64), dimension(:,:)  :: A
            integer(kind=INT32)                             :: i, j
            ! Initialize matrix between -1 and 1
            call random_number(A)
            do i=1, mat_rank
                do j=1, mat_rank
                    A(i,j) = (A(i,j) - 0.5)*2.0d0
                end do
            end do
        end subroutine

        function check(A, mat_rank)
            implicit none
            integer(kind=INT32)                             :: mat_rank
            real(kind=REAL64), dimension(:,:)  :: A
            integer(kind=INT32)                             :: i
            real(kind=REAL64)                               :: check
            do i=1, mat_rank
                check = check + dot_product_idr(A(:,i), A(:,i), mat_rank)
            end do
        end function

        function dot_product_idr(u, v, mat_rank)
            implicit none
            integer(kind=INT32)                             :: mat_rank
            real(kind=REAL64), dimension(:)                 :: u, v
            integer(kind=INT32)                             :: i, j
            real(kind=REAL64)                               :: dot_product_idr
            dot_product_idr = 0.0_real64
            do i=1, mat_rank
                 dot_product_idr = dot_product_idr + u(i)*v(i)
            end do
            
        end function

        subroutine normalize(u, mat_rank)
            implicit none
            integer(kind=INT32)                             :: mat_rank
            real(kind=REAL64), dimension(:)    :: u
            
            integer(kind=INT32)                             :: i, j
            real(kind=real64)                               :: N_inv
            N_inv = 1.0_real64 / sqrt(dot_product_idr(u, u, mat_rank))
            do i=1, mat_rank
                u(i) = u(i) * N_inv
            end do
        end subroutine

end program
