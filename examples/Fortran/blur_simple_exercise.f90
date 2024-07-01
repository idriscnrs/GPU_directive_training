MODULE pixels
    USE OPENACC
    IMPLICIT NONE
    CONTAINS
        SUBROUTINE read_matrix_from_file(FILENAME, PIC, ROWS, COLS)
            IMPLICIT NONE
            CHARACTER(LEN=*), INTENT(IN)                :: FILENAME
            INTEGER(kind=1), DIMENSION(0:), INTENT(OUT) :: PIC
            INTEGER, INTENT(IN)                         :: ROWS, COLS
        
            INTEGER                                     :: FILE_UNIT, I, TOTAL_SIZE, IO_STATUS
            INTEGER(KIND=8)                             :: READ_SIZE
            CHARACTER(LEN=80)                           :: IO_MSG
            OPEN(NEWUNIT=FILE_UNIT, FILE=FILENAME, FORM='UNFORMATTED', ACCESS='stream')
        
            TOTAL_SIZE = ROWS * COLS * 3
            READ(FILE_UNIT, IOSTAT=IO_STATUS, IOMSG=IO_MSG) PIC(0:TOTAL_SIZE-1)
        
            CLOSE(FILE_UNIT)
        END SUBROUTINE read_matrix_from_file  

        SUBROUTINE blur(pic, blurred, rows, cols, passes)
            INTEGER(kind=1),DIMENSION(0:),INTENT(IN)  :: pic
            INTEGER(kind=1),DIMENSION(0:),INTENT(OUT) :: blurred
            INTEGER,DIMENSION(0:4,0:4)                :: coefs
            INTEGER,INTENT(IN)                        :: rows,cols, passes
            INTEGER(kind=4)                           :: i,j,p,i_c,j_c,l,pix
            INTEGER :: my_unit
            coefs(0,:)= (/ 1, 2, 3, 2, 1 /)
            coefs(1,:)= (/ 2, 8, 12, 8, 2 /)
            coefs(2,:)= (/ 3, 12, 16, 12, 3 /)
            coefs(3,:)= (/ 2, 8, 12, 8, 2 /)
            coefs(4,:)= (/ 1, 2, 3, 2, 1 /)

            DO p=1, passes
            DO j=2,cols-3
                DO i=2,rows-3
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
    integer                                      :: passes
    integer  (kind=1), dimension(:), allocatable :: pic,blurred_pic
    character(len=: ), allocatable               :: arg
    integer, dimension(2)                        :: n
 
    rows = 2232
    cols = 4000
    passes = 40

    write(6,"(a23,i7,a2,i7)") "Size of the picture is ",rows," x",cols
    allocate(pic(0:rows*3*cols), blurred_pic(0:rows*3*cols))

    call read_matrix_from_file("pic.rgb", pic, rows, cols)
    call blur(pic, blurred_pic, rows, cols, passes)

    call out_pic(pic, "pic.rgb")
    call out_pic(blurred_pic, "blurred.rgb")

    deallocate(pic, blurred_pic)
END PROGRAM blur_pix
