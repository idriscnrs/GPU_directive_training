MODULE pixels
    USE OPENACC
use omp_lib
    IMPLICIT NONE
    CONTAINS
        SUBROUTINE fill(pic,rows,cols)
            INTEGER(kind=1),DIMENSION(0:),INTENT(OUT) :: pic
            INTEGER,INTENT(IN)                        :: rows,cols
            INTEGER                                   :: i,j, my_unit, val
            
            DO i=0, rows-1
                DO j=0, 3*cols-1
                     val = i+(MOD(j,3))*j+MOD(i,256)
                     pic(i*3*cols+j) = MOD(val,128)
                END DO
            END DO
        END SUBROUTINE fill

        SUBROUTINE blur(pic, blurred, rows, cols)
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN)  :: pic
            INTEGER(kind=1),DIMENSION(0:),INTENT(OUT) :: blurred
            INTEGER,DIMENSION(0:4,0:4)               :: coefs
            INTEGER,INTENT(IN)                        :: rows,cols
            INTEGER(kind=4)                         :: i,j,i_c,j_c,l,pix
            INTEGER :: my_unit
            coefs(0,:)= (/ 1, 2, 3, 3, 1 /)
            coefs(1,:)= (/ 2, 8, 12, 8, 2 /)
            coefs(2,:)= (/ 3, 12, 14, 12, 3 /)
            coefs(3,:)= (/ 2, 8, 12, 8, 2 /)
            coefs(4,:)= (/ 1, 2, 3, 2, 1 /)
            !$acc parallel loop copyin(pic(0:), coefs(0:,0:)) copyout(blurred(0:))
!$omp target teams loop map(to:pic(0:),coefs(0:,0:))&
!$omp map(from:blurred(0:))
            DO i=2,rows-3
                DO j=2,cols-3
                    DO l=0,2
                        pix = 0
                        DO i_c=0,4
                            DO j_c=0,4
                                pix = pix + pic((i+i_c-2)*3*cols+(j+j_c-2)*3+l)*(coefs(i_c,j_c)) 
                            END DO
                        END DO
                        blurred(i*3*cols+j*3+l)=pix/128
                    END DO
                END DO
            END DO
        END SUBROUTINE blur

        SUBROUTINE out_pic(pic, name)
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN) :: pic
            CHARACTER(len=*),INTENT(IN)              :: name  
            INTEGER                                  :: my_unit

            OPEN(NEWUNIT=my_unit, FILE=name, status='replace' ,ACCESS="stream", FORM="unformatted")
            WRITE(my_unit,POS=1) pic 
            CLOSE(my_unit)
        END SUBROUTINE out_pic

end module pixels

PROGRAM blur_pix
    use PIXELS
    implicit none

    integer                                      :: rows, cols, i, long, j, check, numarg
    integer  (kind=1), dimension(:), allocatable :: pic,blurred_pic
    character(len=: ), allocatable               :: arg
    integer, dimension(2)                        :: n
 
    rows = 4000
    cols = 4000

    write(6,"(a23,i7,a2,i7)") "Size of the picture is ",rows," x",cols
    allocate(pic(0:rows*3*cols), blurred_pic(0:rows*3*cols))

    call fill(pic,rows,cols)
    call blur(pic, blurred_pic, rows, cols)

    call out_pic(pic, "pic.rgb")
    call out_pic(blurred_pic, "blurred.rgb")

    deallocate(pic, blurred_pic)
END PROGRAM blur_pix

! Code was translated using: intel-application-migration-tool-for-openacc-to-openmp blur_simple_solution.f90
