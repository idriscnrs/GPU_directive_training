#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

void blur(unsigned char* pic,  unsigned char* blurred, size_t rows, size_t cols)
{
    /**
     * Perform the blurring of the picture
     * @ param pic(in): a pointer to the original picture
     * @ param blurred(out): a pointer to the blurred picture
     */
   size_t i, j, l, i_c, j_c;
   unsigned int pix;
   unsigned char coefs[5][5] = { {1,  4,  6,  4,  1},
                                 {4, 16, 24, 16,  4},
                                 {6, 24, 36, 24,  6},
                                 {4, 16, 24, 16,  4},
                                 {1,  4,  6,  4,  1}};

#pragma acc parallel loop copyin(coefs[:5][:5],pic[:3*rows*cols]) copyout(blurred[:3*rows*cols])
#pragma omp target teams loop map(to:coefs[:5][:5],pic[:3*rows*cols])\
            map(from:blurred[:3*rows*cols])
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
}
void fill(unsigned char* pic, size_t rows, size_t cols)
{
    /**
     * Fill the picture with data
     * @param pic(out): a pointer to the pixel to be blurred
     * @param rows(in) the number of rows in the picture
     * @param cols(in) the number of columns in the picture
     *
     */

    size_t i, j;
#pragma acc parallel loop copyout(pic[0:3*rows*cols])
#pragma omp target teams loop map(from:pic[0:3*rows*cols])
   for (i=0; i < rows; ++i)
      for (j=0; j < 3*cols; ++j)
         pic[i*3*cols+j] = (unsigned char) (i+(j%3)*j+i%256)%256;
    
    
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

int main(void)
{
   size_t rows,cols;

   rows = 4000;
   cols = 4000;

   printf("Size of picture is %d x %d\n", rows, cols); 
   unsigned char* pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));
   unsigned char* blurred_pic = (unsigned char*) malloc(rows*3*cols*sizeof(unsigned char));

   // Create the original picture
   fill(pic, rows, cols);

   // Apply the blurring filter
   blur(pic, blurred_pic, rows, cols);

   out_pic(pic, "pic.rgb", rows, cols);
   out_pic(blurred_pic, "blurred.rgb", rows, cols);

   free(pic);
   free(blurred_pic);

   return 0;
}

// Code was translated using: /home/very/bin/acc2mp -async=nowait -specify-language=C -generate-report blur_simple_solution.c
