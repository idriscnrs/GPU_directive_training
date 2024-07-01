module utils
    use iso_fortran_env, only : REAL64, INT32
    contains
        subroutine normalize_cols(mat, mat_size)
            real    (kind=REAL64), allocatable, dimension(:,:), intent(inout) :: mat
            integer (kind=INT32 )                             , intent(in)    :: mat_size  
            real    (kind=REAL64)                                             :: norm = 0.0_real64
            integer (kind=INT32 )                                             :: i,j
            real    (kind=REAL64), allocatable, dimension(:) :: norms
            !$acc declare device_resident(norms(:))
            allocate(norms(mat_size))
!! Compute the L1 norm of each column
            !$acc parallel loop present(mat(:,:), norms(:))
            do j = 1, mat_size
                norm = 0
                !$acc loop reduction(+:norm)
                do i = 1, mat_size
                    norm = norm + mat(i,j)
                enddo
                norms(j) = norm
            enddo
!! Divide each element of the columns by the L1 norm
            !$acc parallel loop present(mat(:,:), norms(:))
            do j = 1, mat_size
                do i = 1, mat_size
                    mat(i,j) = mat(i,j)/norms(j)
                enddo
            enddo
        end subroutine normalize_cols
end module utils

program normalize
    use utils
    real    (kind=REAL64), allocatable, dimension(:,:)  :: mat
    real    (kind=REAL64)                               :: mat_sum
    integer (kind=INT32)                                :: mat_size=2000
    integer (kind=INT32)                                :: i, j

    allocate(mat(mat_size, mat_size))
    !$acc enter data create(mat)
    call random_number(mat)
    !$acc update device(mat(:,:))
    call normalize_cols(mat, mat_size)
!! Compute the sum of all elements of the matrix
    !$acc parallel loop present(mat(:,:)) reduction(+:mat_sum)
    do j = 1, mat_size
        do i = 1, mat_size
            mat_sum = mat_sum + mat(i,j)
        enddo
    enddo
    !$acc exit data delete(mat)
    deallocate(mat)
    print *, mat_sum, "=", mat_size, "?"
end program normalize
