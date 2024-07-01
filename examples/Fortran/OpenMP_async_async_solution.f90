program prod_mat
    use iso_fortran_env, only : INT32, REAL64
    implicit none
    integer (kind=INT32)               :: rank=5000
    real    (kind=REAL64), allocatable :: A(:,:), B(:,:), C(:,:)
    integer (kind=INT32)               :: i, j, k
    integer (kind=INT32)               :: streamA, streamB, streamC

    streamA = 1
    streamB = 2
    streamC = 3

    call create_mat(A, rank, streamA)
    call create_mat(B, rank, streamB)
    call create_mat(C, rank, streamC)

    call init_mat(A, rank, 3.0_real64 , streamA)
    call init_mat(B, rank, 14.0_real64, streamB)
    call init_mat(C, rank, 0.0_real64 , streamC)

    !$omp target teams distribute parallel do simd collapse(3)
    do j=1, rank
        do k=1, rank
            do i=1, rank
                C(i,j) = C(i,j) + A(i,k)*B(k,j)
            enddo
        enddo
    enddo
    !$omp target exit data map(delete:A,B)
    !$omp target exit data map(from:C)
    print *, "Check that this is close to 42.0:", C(12,12)
    deallocate(A, B, C)
    contains
        subroutine create_mat(mat, rank, stream)
            real   (kind=REAL64), intent(inout), allocatable   :: mat(:,:)
            integer(kind=INT32 ), intent(in)                   :: rank, stream
            allocate(mat(rank,rank))
            !$omp target enter data map(alloc:mat) nowait depend(out:mat)
        end subroutine create_mat

        subroutine init_mat(mat, rank, diag, stream)
            real    (kind=REAL64), intent(inout)   :: mat(:,:)
            real    (kind=REAL64), intent(in)      :: diag
            integer (kind=INT32 ), intent(in)      :: rank, stream
            integer (kind=INT32 )                  :: i, j

            !$omp target teams distribute parallel do simd collapse(2) nowait depend(inout:mat)
            do j=1, rank
                do i=1, rank
                   mat(i,j) = 0.0_real64
                enddo
            enddo

            !$omp target teams distribute parallel do simd nowait depend(in:mat)
            do j=1, rank
                mat(j,j) = diag
            enddo
        end subroutine init_mat
end program prod_mat
