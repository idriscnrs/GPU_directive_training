program loop_configuration
    use ISO_FORTRAN_ENV, only : INT32, REAL64
    use openacc
use omp_lib
    implicit none    
    integer(kind=INT32), parameter        :: n = 500
    integer(kind=INT32), dimension(n,n,n) :: table
    integer(kind=INT32)                   :: ngangs, nworkers, nvectors    
    integer(kind=INT32)                   :: i, j, k

    ngangs   = 450
    nworkers = 4
    nvectors = 16 

    !$acc parallel loop gang num_gangs(ngangs) num_workers(nworkers) vector_length(nvectors) copyout(table(:,:,:))
!$omp target teams loop num_teams(ngangs) thread_limit(nvectors) map(from:table(:,:,:))
    do k = 1, n
        !$acc loop worker
!$omp loop
        do j = 1, n
            !$acc loop vector
!$omp loop 
            do i = 1, n
                table(i,j,k) = i + j*1000 + k*1000*1000
            enddo
        enddo
    enddo

    print *, table(1,1,1), table(n,n,n)
end program loop_configuration      

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp Loop_configuration_example.f90
