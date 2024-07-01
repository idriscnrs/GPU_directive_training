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

    !$acc parallel loop &
    !$acc present(A(:rank,:rank), B(:rank,:rank), C(:rank,:rank)) &
    !$acc gang wait(1,2,3)
    do j=1, rank
        do k=1, rank
            !$acc loop vector
            do i=1, rank
                !$acc atomic update
                C(i,j) = C(i,j) + A(i,k)*B(k,j)
            enddo
        enddo
    enddo
    !$acc exit data delete(A(:rank,:rank), B(:rank,:rank)) copyout(C(:rank,:rank))
    print *, "Check that this is close to 42.0:", C(12,12)
    deallocate(A, B, C)
    contains
        subroutine create_mat(mat, rank, stream)
            real   (kind=REAL64), intent(inout), allocatable   :: mat(:,:)
            integer(kind=INT32 ), intent(in)                   :: rank, stream
            allocate(mat(rank,rank))
            !$acc enter data create(mat(:rank,:rank)) async(stream)
        end subroutine create_mat

        subroutine init_mat(mat, rank, diag, stream)
            real    (kind=REAL64), intent(inout)   :: mat(:,:)
            real    (kind=REAL64), intent(in)      :: diag
            integer (kind=INT32 ), intent(in)      :: rank, stream
            integer (kind=INT32 )                  :: i, j

            !$acc parallel loop collapse(2) async(stream)
            do j=1, rank
                do i=1, rank
                   mat(i,j) = 0.0_real64
                enddo
            enddo

            !$acc parallel loop async(stream)
            do j=1, rank
                mat(j,j) = diag
            enddo
        end subroutine init_mat
end program prod_mat
