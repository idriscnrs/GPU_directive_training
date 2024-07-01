MODULE pixels
    USE OPENACC
use omp_lib
    IMPLICIT NONE
    CONTAINS
        SUBROUTINE fill(pic,rows,cols)
            INTEGER(kind=1),DIMENSION(0:),INTENT(OUT) :: pic
            INTEGER,INTENT(IN)                        :: rows,cols
            INTEGER                                   :: i,j
            !$acc parallel loop present(pic(0:))
!$omp target teams loop 
            DO i=0, rows-1
                !$acc loop
!$omp loop
                DO j=0, 3*cols-1
                     pic(i*3*cols+j) = MOD(i+(MOD(j,3))*j+MOD(i,128),128)
                END DO
            END DO
        END SUBROUTINE fill

        INTEGER(kind=1) FUNCTION weight(pic, x, y, l, cols)
!$omp declare target
            !$acc routine seq
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN) :: pic
            INTEGER,INTENT(IN)                       :: x,y,l,cols
            INTEGER,DIMENSION(0:4,0:4)               :: coefs
            INTEGER                                  :: i,j,pix

            coefs(0,:)= (/ 1, 4, 6, 4, 1 /)
            coefs(1,:)= (/ 4, 16, 24, 16, 4 /)
            coefs(2,:)= (/ 6, 24, 36, 24, 6 /)
            coefs(3,:)= (/ 4, 16, 24, 16, 4 /)
            coefs(4,:)= (/ 1, 4, 6, 4, 1 /)

            pix = 0
            DO i=0,4
                DO j=0,4
                    pix = pix + pic((x+i-2)*3*cols+y*3+l+j-2)*(coefs(i,j)) 
                END DO
            END DO
            weight=pix/256
        END FUNCTION weight

        SUBROUTINE blur(pic, blurred, rows, cols)
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN)  :: pic
            INTEGER(kind=1),DIMENSION(0:),INTENT(OUT) :: blurred
            INTEGER,INTENT(IN)                        :: rows,cols
            INTEGER                                   :: i,j,l
            !$acc parallel loop present(blurred(:), pic(:)) async(2) num_gangs(200)
!$omp target teams loop nowait num_teams(200)
            DO i=2,rows-3
                !$acc loop independent collapse(2)
!$omp loop order(concurrent) collapse(2)
                DO j=2,cols-3
                    DO l=0,2
                        blurred(i*3*cols+j*3+l)=weight(pic, i, j, l, cols)
                    END DO
                END DO
            END DO
        END SUBROUTINE blur

        SUBROUTINE out_pic(pic, name)
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN) :: pic
            CHARACTER(len=*),INTENT(IN)              :: name  
            INTEGER                                  :: my_unit

            OPEN(NEWUNIT=my_unit, FILE=name, ACCESS="stream", FORM="unformatted")
            WRITE(my_unit,POS=1) pic 
            CLOSE(my_unit)
            write(0,*) "Saving a picture "
        END SUBROUTINE out_pic

        INTEGER FUNCTION checksum(pic, rows, cols)
        ! Erreur openacc si INTEGER(kind=1),DIMENSION(0:),INTENT(IN) :: pic
            INTEGER(kind=1),DIMENSION(0:),INTENT(INOUT) :: pic
            INTEGER,INTENT(IN)                          :: rows,cols
            INTEGER                                     :: sum1,sum2,val,i,j,l

            sum1 = 0
            sum2 = 0
            val  = 42424242
            !$acc parallel loop reduction(+:sum1)
!$omp target teams loop reduction(+:sum1)
            DO i=2, rows-3
                sum2=0
                !$acc loop collapse(2) reduction(+:sum2)
!$omp loop reduction(+:sum2) collapse(2)
                DO j=2, cols-3
                    DO l=0,2
                        sum2 = pic(i*3*cols+j*3+l) + sum2
                    END DO
               END DO
               sum1 = MOD(sum2,128) + sum1
            END DO
            PRINT *,"sum1 = ",sum1

           checksum = ISHFT(IEOR(sum1,val),8)
        end function checksum

        subroutine allocation(picture, rows, cols)
            integer  (kind=1), dimension(:), allocatable, intent(inout) :: picture
            integer, intent(in)                                         :: rows, cols

           allocate(picture(0:rows*3*cols-1))
           !$acc enter data create(picture(:))
!$omp target enter data map(alloc:picture(:))
        end subroutine allocation

        subroutine free(picture)
            integer  (kind=1), dimension(:), allocatable, intent(inout) :: picture
            !$acc exit data delete(picture)
!$omp target exit data map(delete:picture)
            deallocate(picture)
        end subroutine free
end module pixels

PROGRAM blur_pix
    use PIXELS
    implicit none

    integer                                      :: rows, cols, i, long, j, check, numarg
    integer  (kind=1), dimension(:), allocatable :: pic,blurred_pic
    character(len=: ), allocatable               :: arg
    integer, dimension(2)                        :: n
 
    numarg = command_argument_count()
    if (numarg .eq. 2) then
        do i=1, COMMAND_ARGUMENT_COUNT()
            call GET_COMMAND_ARGUMENT(NUMBER=i, LENGTH=long)
            allocate(character(len=long) :: arg)
            call GET_COMMAND_ARGUMENT(NUMBER=i, VALUE=arg)
            read(arg,'(i10)') n(i)
            deallocate(arg)
        enddo
        rows = n(1)
        cols = n(2)
    else
        rows = 4000
        cols = 4000
    endif
    write(0,"(a23,i7,a2,i7)") "Size of the picture is ",rows," x",cols
    call allocation(pic, rows, cols)
    call allocation(blurred_pic, rows, cols)

    call fill(pic,rows,cols)
    !$acc update self(pic(:)) async(1)
!$omp target update from(pic(:)) nowait
    call blur(pic, blurred_pic, rows, cols)
    check = checksum(blurred_pic, rows, cols)

    !$acc update self(blurred_pic(rows*3*cols)) wait(2)    
!$omp taskwait
!$omp target update from(blurred_pic(rows*3*cols))
    if (rows*cols <= 16000000) THEN
        !$acc wait(1)  ! Try to remove it and visualize the file pic
!$omp taskwait
        call out_pic(pic, "pic")
        call out_pic(blurred_pic, "blurred.rgb")
    endif
    print *,"Checksum ",check

    call free(pic)
    call free(blurred_pic)
END PROGRAM blur_pix

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp blur.f90
