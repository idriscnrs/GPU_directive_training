#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
/**
 * Apply a gaussian blurring filter to a picture generated on the fly
 *
 * List of functions:
 *   - void blur(unsigned char* pic,  unsigned char* blurred, size_t rows, size_t cols)
 *     the actual filter
 *   - void fill(unsigned char* pic, size_t rows, size_t cols)
 *     generate the original picture
 *   - void out_pic(unsigned char* pic, char* name, size_t rows, size_t cols)
 *     create a .rgb file 
 */

void blur(unsigned char* pic,  unsigned char* blurred, size_t rows, size_t cols, int passes)
{
    /**
     * Perform the blurring of the picture
     * @ param pic(in): a pointer to the original picture
     * @ param blurred(out): a pointer to the blurred picture
     */
   size_t i, j, l, i_c, j_c;
   unsigned char *temp;
   unsigned int pix;
   unsigned char coefs[5][5] = { {1,  4,  6,  4,  1},
                                 {4, 16, 24, 16,  4},
                                 {6, 24, 36, 24,  6},
                                 {4, 16, 24, 16,  4},
                                 {1,  4,  6,  4,  1}};
   for (int pass = 0; pass < passes; ++pass){
      #pragma acc parallel loop copyin(pic[0:rows*3*cols],coefs[:5][:5]) copyout(blurred[0:rows*3*cols])
      for (i=2; i<rows-2; ++i)
         for (j=2; j<cols-2; ++j)
            for (l=0; l<3; ++l)
            {
               pix = 0;
               for (i_c=0; i_c<5; ++i_c)
                  for (j_c=0; j_c<5; ++j_c)
                     pix += (pic[(i+i_c-2)*3*cols+(j+j_c-2)*3+l]
                              *coefs[i_c][j_c]);

               blurred[i*3*cols+j*3+l] = (unsigned char)(pix/256);
            }
      temp = pic;
      pic = blurred;
      blurred = temp;
   }
}

void out_pic(unsigned char* pic, char* name, size_t rows, size_t cols)
{
    /**
     * Output of the picture into a sequence of pixel
     * Use show_rgb(filepath, rows, cols) to display
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     */
   FILE* f = fopen(name, "wb");
   fwrite(pic, sizeof(unsigned char), rows*3*cols, f);
   fclose(f);
}

void read_matrix_from_file(char *filename, unsigned char *pic, int rows, int cols)
{
   /**
    * @brief Reads a 3D matrix from a binary file.
    *
    * This function reads a binary file and stores the data in a 3D matrix.
    * The data is assumed to be stored in binary format and is read in one pass.
    *
    * @param filename The name of the file to read from.
    * @param pic A pointer to a pointer to a pointer to an integer.
    * This is the 3D matrix that will store the data.
    * @param rows The number of rows in the matrix.
    * @param cols The number of columns in the matrix.
    */
   FILE *file = fopen(filename, "rb");

   if (file == NULL)
   {
      printf("Could not open file\n");
      return;
   }

   size_t total_size = rows * cols * 3 * sizeof(unsigned char);
   size_t read_size = fread(pic, sizeof(unsigned char), total_size, file);
   if (read_size != total_size)
   {
      printf("Could not read all values from file\n %ld instead of %ld\n", read_size, total_size);
      return;
   }

   fclose(file);
}

int main(void)
{
   size_t rows,cols;

   rows = 2252;
   cols = 4000;

   printf("Size of picture is %ld x %ld\n", rows, cols); 
   unsigned char* pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));
   unsigned char* blurred_pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));

   // Reads the original picture
   read_matrix_from_file("pic.rgb", pic, rows, cols);

   // Apply the blurring filter
   int passes = 40;
   blur(pic, blurred_pic, rows, cols, passes);

   out_pic(pic, "pic_read.rgb", rows, cols);
   out_pic(blurred_pic, "blurred.rgb", rows, cols);

   free(pic);
   free(blurred_pic);

   return 0;
}
